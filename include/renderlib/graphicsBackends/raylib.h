#pragma once

#include "../itf/graphicsBackend.h"
#include "raylib.h"
#include "raymath.h"
#include "renderlib/primitives.h"

namespace renderlib {

inline Color toColor(vec4i vec)
{
    return Color{
        (unsigned char)vec[0], (unsigned char)vec[1], (unsigned char)vec[2],
        (unsigned char)vec[3]};
}

template <typename T>
inline auto toVector(T vec)
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

class RaylibGraphicsBackend : public itf::GraphicsBackend
{
public:
    void drawBox(vec3 size, PrimitiveColors const& colors, mat4 const& xform);
    void drawBox(Box const& box);
    void drawSphere(
        real radius, PrimitiveColors const& colors, mat4 const& xform);
    void drawSphere(Sphere const& sphere);
    void drawCircle(
        real radius, PrimitiveColors const& colors, real thickness,
        mat4 const& xform);
    void drawCircle(Circle const& circle);
    void drawGrid(int slices, real cellSize, mat4 const& xform);
    void drawGrid(Grid3D const& grid);

    void open(vec2i size, string title) override;
    void close() override;
    bool shouldClose() override;
    void clear() override;
    void endFrame() override;
    void render(Entities* entities) override;
    real dt() override;
};
}  // namespace renderlib