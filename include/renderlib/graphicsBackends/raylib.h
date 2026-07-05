#pragma once

#include "../itf/graphicsBackend.h"

namespace renderlib {
class RaylibGraphicsBackend : public itf::GraphicsBackend
{
public:
    void open(vec2i size, string title) override;
    void close() override;
    bool shouldClose() override;
    void clear() override;
    void endFrame() override;
    void render(Entities* entities) override;
    real dt() override;
};
}  // namespace renderlib