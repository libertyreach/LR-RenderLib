#include <iostream>
#include <raylib.h>
#include "rlgl.h"
#include "raymath.h"

#include <renderlib/graphicsBackends/raylib.h>
#include <renderlib/camera.h>
#include <renderlib/primitives.h>

#include <renderlib/entities.h>

using namespace renderlib;

Color toColor(vec4i vec)
{
    return Color{
        (unsigned char)vec[0], (unsigned char)vec[1], (unsigned char)vec[2],
        (unsigned char)vec[3]};
}

template <typename T>
auto toVector(T vec)
{
    if constexpr (std::is_same_v<T, vec2i> || std::is_same_v<T, vec2>)
    {
        return Vector2{vec[0], vec[1]};
    }
    else if constexpr (std::is_same_v<T, vec3i> || std::is_same_v<T, vec3>)
    {
        return Vector3{vec[0], vec[1], vec[2]};
    }
    else if constexpr (std::is_same_v<T, vec4i> || std::is_same_v<T, vec4>)
    {
        return Vector4{vec[0], vec[1], vec[2], vec[3]};
    }
    else { throw std::runtime_error("Invalid vector conversion"); }
}

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

        // Draw a box under its world transform: push the composed matrix and
        // draw the cube at the local origin so rotation/parenting are honored.
        auto drawBox = [](Box const& box) {
            rlPushMatrix();
            rlMultMatrixf(box.entity->worldMatrix.m.data());
            DrawCubeV(
                Vector3{0.0f, 0.0f, 0.0f}, toVector(box.size),
                toColor(box.color));
            rlPopMatrix();
        };

        entities->registry()->view<const Box>().each([&](Box const& box) {
            if (box.color[3] < 255)
            {
                commands.emplace_back(
                    box.entity, [box, drawBox] { drawBox(box); });
            }
            else { drawBox(box); }
        });

        // A sphere is rotation-invariant, so its world position is enough.
        auto drawSphere = [](Sphere const& sphere) {
            DrawSphere(
                toVector(sphere.entity->worldPosition()), sphere.radius,
                toColor(sphere.color));
        };

        entities->registry()->view<const Sphere>().each(
            [&](Sphere const& sphere) {
                if (sphere.color[3] < 255)
                {
                    commands.emplace_back(sphere.entity, [sphere, drawSphere] {
                        drawSphere(sphere);
                    });
                }
                else { drawSphere(sphere); }
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