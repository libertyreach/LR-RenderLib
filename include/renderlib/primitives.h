#pragma once

#include "component.h"
#include "types.h"

namespace renderlib {

struct Box : public Component
{
    vec4i color;
    vec3 size;
};

struct Sphere : public Component
{
    vec4i color;
    real radius;
};

struct Circle : public Component
{
    vec4i color;
    real radius;
    bool filled;
    real thickness;
};

struct Text2D : public Component
{
    vec4i color;
    vec2 position;
    string text;
    int fontSize;
};

struct Text3D : public Component
{
    vec4i color;
    string text;
    int fontSize;
};

}  // namespace renderlib