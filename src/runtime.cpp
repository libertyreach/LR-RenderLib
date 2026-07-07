#include <renderlib/runtime.h>

using namespace renderlib;

Runtime::Runtime(
    std::shared_ptr<itf::GraphicsBackend> graphics, vec2i size, string title)
    : m_size(size), m_title(title), m_graphics(std::move(graphics))
{
    m_running.store(true);
    m_thread = std::thread(&Runtime::run, this);
}

Runtime::~Runtime()
{
    m_running.store(false);
    if (m_thread.joinable()) m_thread.join();
}

void Runtime::modify(std::function<void(Entities*)> lambda)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    lambda(&m_entities);
}

void Runtime::run()
{
    m_graphics->open(
        m_size, m_title);  // window + context now created on THIS thread
    while (m_running.load() && !m_graphics->shouldClose())
    {
        real dt = m_graphics->dt();
        {
            // One lock for the whole tick: guards m_entities against modify()
            // and m_systems against addSystem() while we iterate.
            std::lock_guard<std::mutex> lock(m_mutex);

            // Update before rendering so the frame shows the current state.
            for (const auto& [id, system] : m_systems)
            {
                system->onBeforeUpdate(&m_entities);
            }
            for (const auto& [id, system] : m_systems)
            {
                system->onUpdate(&m_entities, dt);
            }
            for (const auto& [id, system] : m_systems)
            {
                system->onAfterUpdate(&m_entities);
            }

            // Compose the scene hierarchy into world transforms before drawing.
            m_entities.updateTransforms();

            m_graphics->clear();
            m_graphics->render(&m_entities);
            m_graphics->customRenderer(m_graphics.get(), &m_entities);
            m_graphics->endFrame();
        }
    }
    m_graphics->close();
    m_running.store(false);
}