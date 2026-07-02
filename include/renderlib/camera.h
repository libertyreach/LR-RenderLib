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
    float width = 10.0f;
};

struct OrthographicZoom : public Component
{
    float speed = 1.0f;  // fraction of the view zoomed per wheel tick
};

struct OrthographicPan : public Component
{
    float speed = 1.0f;  // drag sensitivity; 1.0 tracks the cursor exactly
};

struct PerspectiveCamera : public Camera
{
    float fov = 45.0f;
};

}  // namespace renderlib