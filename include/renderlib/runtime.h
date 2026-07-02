#pragma once

#include <atomic>

#include <entt/entt.hpp>
#include <mutex>
#include <thread>

#include "itf/graphicsBackend.h"
#include "system.h"

namespace renderlib {
class Runtime
{
public:
    Runtime(
        std::unique_ptr<itf::GraphicsBackend> graphics, vec2i size,
        string title);
    ~Runtime();

    void modify(std::function<void(Entities*)> lambda);

    bool running() { return m_running.load(); }

    template <IsSystem T>
    bool hasSystem()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_systems.find(typeid(T).hash_code()) != m_systems.end();
    }

    template <IsSystem T>
    T* getSystem()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_systems.find(typeid(T).hash_code());
        return it == m_systems.end() ? nullptr
                                     : static_cast<T*>(it->second.get());
    }

    template <IsSystem T, typename... Args>
    T* addSystem(Args&&... args)
    {
        // Inlined find/insert (not via has/getSystem) so we hold m_mutex once
        // and don't recursively lock it — std::mutex is not reentrant.
        std::lock_guard<std::mutex> lock(m_mutex);
        auto hash = typeid(T).hash_code();
        auto it = m_systems.find(hash);
        if (it != m_systems.end()) return static_cast<T*>(it->second.get());
        it = m_systems
                 .insert(std::make_pair(
                     hash, std::make_unique<T>(std::forward<Args>(args)...)))
                 .first;
        return static_cast<T*>(it->second.get());
    }

    Entities* entities() { return &m_entities; }

private:
    vec2i m_size;
    string m_title;
    std::unique_ptr<itf::GraphicsBackend> m_graphics;
    std::atomic<bool> m_running{false};
    std::mutex m_mutex;
    std::thread m_thread;
    Entities m_entities;
    map<size_t, std::unique_ptr<System>> m_systems;

    void run();
};
}  // namespace renderlib