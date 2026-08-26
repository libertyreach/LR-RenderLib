#pragma once

#include "types.h"
#include "component.h"

namespace renderlib {

struct Camera : public Component
{
    vec3 target = {0, 0, 0};
    vec3 up = {0, 1, 0};
};

struct OrthographicCamera : public Camera
{
    real width = 10.0f;
};

struct OrthographicZoom : public Component
{
    real speed = 1.0f;  // fraction of the view zoomed per wheel tick
};

struct OrthographicPan : public Component
{
    real speed = 1.0f;  // drag sensitivity; 1.0 tracks the cursor exactly
};

struct PerspectiveCamera : public Camera
{
    real fov = 45.0f;

    // Which world axis points up. renderlib's convention is Z-up: X/Y is the
    // flat plane and Z is the vertical. raylib is Y-up, so the backend feeds
    // this axis to raylib's camera up vector (instead of Camera::up), and the
    // fly camera uses it for vertical movement and to keep the horizon level.
    vec3 upAxis = {0, 0, 1};
};

struct FlyCamera : public Component
{
    real speed = 2500;
    real fastSpeed = 5000;

    // FPS-style mouse look, active while the middle mouse button is held.
    real lookSensitivity = 0.15f;  // degrees of rotation per pixel of movement
    real maxPitch = 89.0f;  // degrees; keeps the view off the up axis poles
    bool invertLookY = false;
};

}  // namespace renderlib