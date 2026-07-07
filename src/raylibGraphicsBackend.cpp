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

// Applies mouse-driven pan/zoom to an orthographic camera when its entity
// carries the OrthographicPan / OrthographicZoom control components.
//
// Assumes the conventional orthographic setup: looking down -Z with +Y up and
// no roll, so screen axes map directly to world X/Y and the screen centre shows
// the camera's target. cam.width is the visible *height* in world units (raylib
// uses an orthographic camera's fovy as the vertical extent).
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

void RaylibGraphicsBackend::drawBox(Box const& box)
{
    rlPushMatrix();
    rlMultMatrixf(box.entity->worldMatrix.m.data());
    DrawCubeV(
        Vector3{0.0f, 0.0f, 0.0f}, toVector(box.size), toColor(box.color));
    rlPopMatrix();
}

void RaylibGraphicsBackend::drawSphere(Sphere const& sphere)
{
    DrawSphere(
        toVector(sphere.entity->worldPosition()), sphere.radius,
        toColor(sphere.color));
}

void RaylibGraphicsBackend::drawCircle(Circle const& circle)
{
    rlPushMatrix();
    rlMultMatrixf(circle.entity->worldMatrix.m.data());

    Color color = toColor(circle.color);
    const int segments = 64;
    const float step = 2.0f * PI / segments;
    const float r = circle.radius;

    rlBegin(RL_TRIANGLES);
    rlColor4ub(color.r, color.g, color.b, color.a);
    if (circle.filled)
    {
        for (int i = 0; i < segments; i++)
        {
            float a0 = i * step;
            float a1 = (i + 1) * step;
            rlVertex3f(0.0f, 0.0f, 0.0f);
            rlVertex3f(cosf(a0) * r, sinf(a0) * r, 0.0f);
            rlVertex3f(cosf(a1) * r, sinf(a1) * r, 0.0f);
        }
    }
    else
    {
        float inner = r - circle.thickness;
        if (inner < 0.0f) inner = 0.0f;
        for (int i = 0; i < segments; i++)
        {
            float a0 = i * step;
            float a1 = (i + 1) * step;
            float c0 = cosf(a0), s0 = sinf(a0);
            float c1 = cosf(a1), s1 = sinf(a1);

            // Two triangles forming the quad between inner and outer
            // radius for this segment.
            rlVertex3f(c0 * inner, s0 * inner, 0.0f);
            rlVertex3f(c0 * r, s0 * r, 0.0f);
            rlVertex3f(c1 * r, s1 * r, 0.0f);

            rlVertex3f(c0 * inner, s0 * inner, 0.0f);
            rlVertex3f(c1 * r, s1 * r, 0.0f);
            rlVertex3f(c1 * inner, s1 * inner, 0.0f);
        }
    }
    rlEnd();

    rlPopMatrix();
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
        entities->registry()->view<const PerspectiveCamera>().each(
            [&](PerspectiveCamera const& perspective) {
                if (foundCamera) std::cout << "Camera already set!\n";
                camera3D = true;
                camera.position =
                    toVector(perspective.entity->transform.position);
                camera.target = toVector(perspective.target);
                camera.up = toVector(perspective.up);
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

        entities->registry()->view<const Box>().each([&](Box const& box) {
            if (box.color[3] < 255)
            {
                commands.emplace_back(box.entity, [&, box] { drawBox(box); });
            }
            else { drawBox(box); }
        });

        entities->registry()->view<const Sphere>().each(
            [&](Sphere const& sphere) {
                if (sphere.color[3] < 255)
                {
                    commands.emplace_back(
                        sphere.entity, [&, sphere] { drawSphere(sphere); });
                }
                else { drawSphere(sphere); }
            });

        entities->registry()->view<const Circle>().each(
            [&](Circle const& circle) {
                if (circle.color[3] < 255)
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