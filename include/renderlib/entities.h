#pragma once

#include "types.h"
#include "component.h"
#include <entt/entt.hpp>

namespace renderlib {

// Local transform, relative to the entity's parent. World-space values are
// derived by Entities::updateTransforms() and cached in Entity::worldMatrix.
struct Transform
{
    vec3 position = {0, 0, 0};
    vec3 rotation = {0, 0, 0};  // Euler angles in degrees
};

class Entity
{
public:
    friend class Entities;

    string name;
    Transform transform;  // local
    Entity* parent = nullptr;
    list<Entity*> children;

    // Composed local->world transform, refreshed each frame by
    // Entities::updateTransforms(). Identity for entities with no parent.
    mat4 worldMatrix = mat4::identity();

    // Convenience: this entity's origin in world space.
    vec3 worldPosition() const { return worldMatrix.translation(); }

    Entity(entt::registry* registry);

    template <IsComponent T, typename... Args>
    T* addComponent(Args&&... args)
    {
        if (hasComponent<T>()) { return getComponent<T>(); }
        auto component =
            &m_registry->emplace<T>(m_id, std::forward<Args>(args)...);
        component->entity = this;
        return component;
    }

    template <IsComponent T>
    T* getComponent()
    {
        return hasComponent<T>() ? &m_registry->get<T>(m_id) : nullptr;
    }

    template <IsComponent T>
    bool hasComponent()
    {
        return m_registry->all_of<T>(m_id);
    }

    entt::entity id() { return m_id; }

private:
    entt::entity m_id;
    entt::registry* m_registry;
};

class Entities
{
public:
    Entity* get(entt::entity id);
    Entity* add(string name = "");
    Entity* add(Entity* parent, string name = "");
    void remove(Entity* entity);
    entt::registry* registry();

    // Recompute every entity's world transform by composing parent->child down
    // the hierarchy. Call once per frame after local transforms are updated and
    // before rendering.
    void updateTransforms();

private:
    entt::registry m_registry;
    map<entt::entity, std::unique_ptr<Entity>> m_entities;

    void updateWorld(Entity* entity, mat4 const& parentWorld);
};

}  // namespace renderlib