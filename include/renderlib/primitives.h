#pragma once

#include "component.h"
#include "types.h"

namespace renderlib {

enum class ColorMode
{
    FillOnly,
    BorderOnly,
    FillAndBorder,
};

struct PrimitiveColors
{
    ColorMode mode;
    vec4i fillColor;
    vec4i borderColor;
};

struct Box : public Component
{
    PrimitiveColors colors;
    vec3 size;
};

struct Sphere : public Component
{
    PrimitiveColors colors;
    real radius;
};

struct Circle : public Component
{
    PrimitiveColors colors;
    real radius;
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

struct Grid3D : public Component
{
    int slices;
    real cellSize;
};

}  // namespace renderlib