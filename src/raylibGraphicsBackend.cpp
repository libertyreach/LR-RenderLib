#include <iostream>
#include <raylib.h>
#include "rlgl.h"


#include <renderlib/graphicsBackends/raylib.h>
#include <renderlib/camera.h>
#include <renderlib/primitives.h>

#include <renderlib/entities.h>

using namespace renderlib;

void RaylibGraphicsBackend::clear()
{
    BeginDrawing();
    ClearBackground(toColor(clearColor));
}

real RaylibGraphicsBackend::dt() { return GetFrameTime(); }

struct DrawCommand
{
    Entity* entity;
    std::function<void()> command;
};

// Rotate `v` about the unit vector `axis` by `radians` (Rodrigues' formula).
static vec3 rotateAbout(vec3 const& v, vec3 const& axis, real radians)
{
    real c = std::cos(radians);
    real s = std::sin(radians);
    return v * c + axis.cross(v) * s + axis * (axis.dot(v) * (1 - c));
}

// The axis orthogonal to forward used for strafing and as the pitch axis. Kept
// orthogonal to the world up axis so neither looking nor strafing ever rolls
// the camera.
static vec3 rightOf(vec3 const& forward, vec3 const& upAxis)
{
    vec3 right = forward.cross(upAxis);
    if (right.magnitudeSq() < (real)1e-12)
    {
        // Looking straight up or down the up axis: pick any axis orthogonal
        // to forward so the basis stays well defined.
        right = forward.cross(
            std::abs(forward[0]) < (real)0.9 ? vec3{1, 0, 0} : vec3{0, 1, 0});
    }
    return right.normalized();
}

static void applyPerspectiveCameraControls(PerspectiveCamera& cam)
{
    if (auto* flyCam = cam.entity->getComponent<FlyCamera>())
    {
        Entity* entity = cam.entity;

        // Camera basis. Forward is the view direction; right is orthogonal to
        // both forward and the world up axis (cam.upAxis, Z in renderlib's
        // convention), so strafing never rolls the camera.
        vec3 upAxis = cam.upAxis.normalized();
        if (upAxis.magnitudeSq() == 0) upAxis = vec3{0, 0, 1};

        vec3 offset = cam.target - entity->transform.position;
        real targetDistance = offset.magnitude();

        vec3 forward = offset;
        if (targetDistance == 0)
        {
            // No view direction to work from: any axis off the up axis will do.
            forward = std::abs(upAxis[0]) < (real)0.9 ? vec3{1, 0, 0}
                                                     : vec3{0, 1, 0};
            targetDistance = 1;
        }
        forward.normalize();

        vec3 right = rightOf(forward, upAxis);

        // FPS-style mouse look while the middle mouse button is held: yaw about
        // the world up axis, pitch about the camera's right axis.
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            Vector2 mouse = GetMouseDelta();
            real sensitivity = flyCam->lookSensitivity * (real)DEG2RAD;

            if (mouse.x != 0.0f)
            {
                // Dragging right turns right, i.e. away from the right axis.
                forward = rotateAbout(forward, upAxis, -mouse.x * sensitivity)
                              .normalized();
                right = rightOf(forward, upAxis);
            }

            if (mouse.y != 0.0f)
            {
                real dPitch = -mouse.y * sensitivity;
                if (flyCam->invertLookY) dPitch = -dPitch;

                // Clamp the resulting pitch so the view never tips over the
                // pole (which would flip the horizon).
                real limit = flyCam->maxPitch * (real)DEG2RAD;
                real pitch =
                    std::asin(clamp(forward.dot(upAxis), (real)-1, (real)1));
                dPitch = clamp(pitch + dPitch, -limit, limit) - pitch;

                forward = rotateAbout(forward, right, dPitch).normalized();
            }

            // Look rotates about the camera, so the target follows the view at
            // its original distance.
            cam.target = entity->transform.position + forward * targetDistance;
        }

        vec3 move;
        if (IsKeyDown(KEY_W)) move += forward;
        if (IsKeyDown(KEY_S)) move -= forward;
        if (IsKeyDown(KEY_D)) move += right;
        if (IsKeyDown(KEY_A)) move -= right;
        // Vertical movement follows the world up axis, not the view, so E/Q
        // always climb and descend regardless of pitch.
        if (IsKeyDown(KEY_E)) move += upAxis;
        if (IsKeyDown(KEY_Q)) move -= upAxis;

        if (move.magnitudeSq() > 0)
        {
            real speed =
                (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
                    ? flyCam->fastSpeed
                    : flyCam->speed;

            // Normalized so diagonal movement isn't faster than axial.
            vec3 delta = move.normalized() * (speed * (real)GetFrameTime());

            // Translate position and target together to keep the view
            // direction unchanged while flying.
            entity->transform.position += delta;
            cam.target += delta;
        }
    }
}

static void applyOrthographicControls(OrthographicCamera& cam)
{
    Entity* entity = cam.entity;
    const float screenW = (float)GetScreenWidth();
    const float screenH = (float)GetScreenHeight();

    // Zoom (mouse wheel), keeping the world point under the cursor fixed.
    if (auto* zoom = entity->getComponent<OrthographicZoom>())
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
        {
            Vector2 mouse = GetMousePosition();

            // World point under the cursor before the zoom.
            float wppBefore = cam.width / screenH;
            float worldX =
                cam.target[0] + (mouse.x - screenW * 0.5f) * wppBefore;
            float worldY =
                cam.target[1] - (mouse.y - screenH * 0.5f) * wppBefore;

            // Proportional zoom; scrolling up (positive wheel) zooms in.
            cam.width *= (1.0f - wheel * 0.1f * zoom->speed);
            if (cam.width < 0.01f) cam.width = 0.01f;

            // Re-centre so that same world point stays under the cursor.
            float wppAfter = cam.width / screenH;
            float centerX = worldX - (mouse.x - screenW * 0.5f) * wppAfter;
            float centerY = worldY + (mouse.y - screenH * 0.5f) * wppAfter;

            entity->transform.position[0] = centerX;
            entity->transform.position[1] = centerY;
            cam.target[0] = centerX;
            cam.target[1] = centerY;
        }
    }

    // Pan: drag with the middle mouse button, world tracks the cursor 1:1
    // (scaled by pan->speed).
    if (auto* pan = entity->getComponent<OrthographicPan>())
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            Vector2 delta = GetMouseDelta();
            float wpp = cam.width / screenH;
            float dx = delta.x * wpp * pan->speed;
            float dy = delta.y * wpp * pan->speed;

            // Move the camera opposite the drag so content follows the cursor.
            entity->transform.position[0] -= dx;
            entity->transform.position[1] += dy;
            cam.target[0] -= dx;
            cam.target[1] += dy;
        }
    }
}

static bool hasFill(PrimitiveColors const& colors)
{
    return colors.mode == ColorMode::FillOnly ||
           colors.mode == ColorMode::FillAndColor;
}

static bool hasBorder(PrimitiveColors const& colors)
{
    return colors.mode == ColorMode::BorderOnly ||
           colors.mode == ColorMode::FillAndColor;
}

// Whether any part that will actually be drawn is translucent, and so has to
// be sorted back-to-front with the other translucent primitives.
static bool isTranslucent(PrimitiveColors const& colors)
{
    return (hasFill(colors) && colors.fillColor[3] < 255) ||
           (hasBorder(colors) && colors.borderColor[3] < 255);
}

void RaylibGraphicsBackend::drawBox(
    vec3 size, PrimitiveColors const& colors, mat4 const& xform)
{
    rlPushMatrix();
    rlMultMatrixf(xform.m.data());

    const Vector3 origin{0.0f, 0.0f, 0.0f};
    if (hasFill(colors))
    {
        DrawCubeV(origin, toVector(size), toColor(colors.fillColor));
    }
    if (hasBorder(colors))
    {
        DrawCubeWiresV(origin, toVector(size), toColor(colors.borderColor));
    }

    rlPopMatrix();
}

void RaylibGraphicsBackend::drawBox(Box const& box)
{
    drawBox(box.size, box.colors, box.entity->worldMatrix);
}

void RaylibGraphicsBackend::drawSphere(
    real radius, PrimitiveColors const& colors, mat4 const& xform)
{
    rlPushMatrix();
    rlMultMatrixf(xform.m.data());

    const Vector3 origin{0.0f, 0.0f, 0.0f};
    const int rings = 16;
    const int slices = 16;

    if (hasFill(colors))
    {
        DrawSphere(origin, radius, toColor(colors.fillColor));
    }
    if (hasBorder(colors))
    {
        DrawSphereWires(
            origin, radius, rings, slices, toColor(colors.borderColor));
    }

    rlPopMatrix();
}

void RaylibGraphicsBackend::drawSphere(Sphere const& sphere)
{
    drawSphere(sphere.radius, sphere.colors, sphere.entity->worldMatrix);
}

void RaylibGraphicsBackend::drawCircle(
    real radius, PrimitiveColors const& colors, real thickness,
    mat4 const& xform)
{
    const bool fill = hasFill(colors);
    const bool border = hasBorder(colors);
    if (!fill && !border) return;

    rlPushMatrix();
    rlMultMatrixf(xform.m.data());

    const int segments = 64;
    const float step = 2.0f * PI / segments;

    // The border occupies the outer `thickness` of the radius; the fill stops
    // where it begins so the two never overlap (coplanar geometry with the
    // depth mask off would otherwise z-fight).
    float inner = radius;
    if (border)
    {
        inner = radius - (float)thickness;
        if (inner < 0.0f) inner = 0.0f;
    }

    rlBegin(RL_TRIANGLES);

    if (fill)
    {
        Color fillColor = toColor(colors.fillColor);
        rlColor4ub(fillColor.r, fillColor.g, fillColor.b, fillColor.a);
        for (int i = 0; i < segments; i++)
        {
            float a0 = i * step;
            float a1 = (i + 1) * step;
            rlVertex3f(0.0f, 0.0f, 0.0f);
            rlVertex3f(cosf(a0) * inner, sinf(a0) * inner, 0.0f);
            rlVertex3f(cosf(a1) * inner, sinf(a1) * inner, 0.0f);
        }
    }

    if (border)
    {
        Color borderColor = toColor(colors.borderColor);
        rlColor4ub(
            borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        for (int i = 0; i < segments; i++)
        {
            float a0 = i * step;
            float a1 = (i + 1) * step;
            float c0 = cosf(a0), s0 = sinf(a0);
            float c1 = cosf(a1), s1 = sinf(a1);

            // Two triangles forming the quad between inner and outer
            // radius for this segment.
            rlVertex3f(c0 * inner, s0 * inner, 0.0f);
            rlVertex3f(c0 * radius, s0 * radius, 0.0f);
            rlVertex3f(c1 * radius, s1 * radius, 0.0f);

            rlVertex3f(c0 * inner, s0 * inner, 0.0f);
            rlVertex3f(c1 * radius, s1 * radius, 0.0f);
            rlVertex3f(c1 * inner, s1 * inner, 0.0f);
        }
    }

    rlEnd();

    rlPopMatrix();
}

void RaylibGraphicsBackend::drawCircle(Circle const& circle)
{
    drawCircle(
        circle.radius, circle.colors, circle.thickness,
        circle.entity->worldMatrix);
}

void RaylibGraphicsBackend::drawGrid(
    int slices, real cellSize, mat4 const& xform)
{
    rlPushMatrix();
    rlMultMatrixf(xform.m.data());
    // raylib draws the grid in its own XZ plane (it is Y-up); rotating it a
    // quarter turn about X lands it on the XY plane, which is renderlib's flat
    // ground plane. Rotate the entity itself to place it anywhere else.
    rlMultMatrixf(mat4::rotateEuler(vec3{90, 0, 0}).m.data());
    DrawGrid(slices, (float)cellSize);
    rlPopMatrix();
}

void RaylibGraphicsBackend::drawGrid(Grid3D const& grid)
{
    drawGrid(grid.slices, grid.cellSize, grid.entity->worldMatrix);
}

void RaylibGraphicsBackend::render(Entities* entities)
{
    bool foundCamera = false;
    bool camera3D = false;
    Camera3D camera;

    {
        entities->registry()->view<OrthographicCamera>().each(
            [&](OrthographicCamera& orth) {
                if (foundCamera) std::cout << "Camera already set!\n";
                applyOrthographicControls(orth);
                camera.position = toVector(orth.entity->transform.position);
                camera.target = toVector(orth.target);
                camera.up = toVector(orth.up);
                camera.fovy = orth.width;
                camera.projection = CAMERA_ORTHOGRAPHIC;
                foundCamera = true;
            });
    }

    {
        entities->registry()->view<PerspectiveCamera>().each(
            [&](PerspectiveCamera& perspective) {
                if (foundCamera) std::cout << "Camera already set!\n";
                applyPerspectiveCameraControls(perspective);
                camera3D = true;
                camera.position =
                    toVector(perspective.entity->transform.position);
                camera.target = toVector(perspective.target);
                // raylib is Y-up; feeding it the world up axis is what lets a
                // Z-up scene render upright.
                camera.up = toVector(perspective.upAxis);
                camera.fovy = perspective.fov;
                camera.projection = CAMERA_PERSPECTIVE;
                foundCamera = true;
            });
    }

    if (!foundCamera)
    {
        std::cout << "Failed to find camera - skipping rendering\n";
        EndDrawing();
        return;
    }

    list<DrawCommand> commands;

    {
        BeginMode3D(camera);

        // Grids first: they are opaque and sit behind everything else.
        entities->registry()->view<const Grid3D>().each(
            [&](Grid3D const& grid) { drawGrid(grid); });

        entities->registry()->view<const Box>().each([&](Box const& box) {
            if (isTranslucent(box.colors))
            {
                commands.emplace_back(box.entity, [&, box] { drawBox(box); });
            }
            else { drawBox(box); }
        });

        entities->registry()->view<const Sphere>().each(
            [&](Sphere const& sphere) {
                if (isTranslucent(sphere.colors))
                {
                    commands.emplace_back(
                        sphere.entity, [&, sphere] { drawSphere(sphere); });
                }
                else { drawSphere(sphere); }
            });

        entities->registry()->view<const Circle>().each(
            [&](Circle const& circle) {
                if (isTranslucent(circle.colors))
                {
                    commands.emplace_back(
                        circle.entity, [&, circle] { drawCircle(circle); });
                }
                else { drawCircle(circle); }
            });

        std::sort(
            commands.begin(), commands.end(),
            [&](DrawCommand const& a, DrawCommand const& b) {
                if (camera3D)
                {
                    return Vector3DistanceSqr(
                               camera.position,
                               toVector(a.entity->worldPosition())) >
                           Vector3DistanceSqr(
                               camera.position,
                               toVector(b.entity->worldPosition()));
                }
                else
                {
                    return camera.position.z - a.entity->worldPosition()[2] >
                           camera.position.z - b.entity->worldPosition()[2];
                }
            });

        rlDisableDepthMask();
        for (const auto& cmd : commands) cmd.command();
        rlEnableDepthMask();

        EndMode3D();
    }

    // Text is drawn in screen space, after the 3D scene.
    {
        entities->registry()->view<const Text3D>().each(
            [&](Text3D const& text) {
                Vector2 screenPos = GetWorldToScreen(
                    toVector(text.entity->worldPosition()), camera);
                DrawText(
                    text.text.c_str(), (int)screenPos.x, (int)screenPos.y,
                    text.fontSize, toColor(text.color));
            });

        entities->registry()->view<const Text2D>().each(
            [&](Text2D const& text) {
                DrawText(
                    text.text.c_str(), (int)text.position[0],
                    (int)text.position[1], text.fontSize, toColor(text.color));
            });
    }
}

void RaylibGraphicsBackend::endFrame() { EndDrawing(); }

void RaylibGraphicsBackend::open(vec2i size, string title)
{
    InitWindow(size[0], size[1], title.c_str());
    SetTargetFPS(60);
}

bool RaylibGraphicsBackend::shouldClose() { return WindowShouldClose(); }

void RaylibGraphicsBackend::close() { return CloseWindow(); }