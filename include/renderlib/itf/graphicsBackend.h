#pragma once

#include <entt/entt.hpp>

#include "../types.h"
#include "renderlib/entities.h"

namespace renderlib::itf {
struct GraphicsBackend
{
    std::function<void(GraphicsBackend*, Entities* entities)> customRenderer =
        [](auto gfx, auto entt) {};

    vec4i clearColor = vec4i{{0, 0, 0, 255}};

    virtual void open(vec2i size, string title) = 0;
    virtual void close() = 0;
    virtual bool shouldClose() = 0;
    virtual void clear() = 0;
    virtual void endFrame() = 0;
    virtual void render(Entities* entities) = 0;
    virtual real dt() = 0;
};
}  // namespace renderlib::itf