#include <renderlib/entities.h>
#include <algorithm>
#include <format>

using namespace renderlib;

Entity::Entity(entt::registry* registry)
    : m_id(registry->create()), m_registry(registry)
{}

Entity* Entities::get(entt::entity id)
{
    return m_entities.find(id) == m_entities.end() ? nullptr
                                                   : m_entities.at(id).get();
}

Entity* Entities::add(string name)
{
    auto entity = std::make_unique<Entity>(&m_registry);
    auto id = entity->id();
    entity->name = name.empty()
                       ? std::format("Entity {}", (std::uint32_t)entity->id())
                       : name;
    m_entities.insert(std::make_pair(entity->id(), std::move(entity)));
    return get(id);
}

Entity* Entities::add(Entity* parent, string name)
{
    auto entity = add(name);
    entity->parent = parent;
    parent->children.push_back(entity);
    return entity;
}

void Entities::remove(Entity* entity)
{
    if (!entity) return;

    // Copy the child list: remove() erases entities from m_entities as it
    // recurses, and we destroy `entity` (and its children vector) at the end.
    auto children = entity->children;
    for (auto* child : children) { remove(child); }

    // Detach from the parent so we don't leave a dangling pointer behind.
    if (entity->parent)
    {
        auto& siblings = entity->parent->children;
        siblings.erase(
            std::remove(siblings.begin(), siblings.end(), entity),
            siblings.end());
    }

    auto id = entity->id();
    m_registry.destroy(id);
    m_entities.erase(id);
}

void Entities::updateTransforms()
{
    for (auto& [id, entity] : m_entities)
    {
        // Roots seed the recursion; children are reached through them.
        if (entity->parent == nullptr)
        {
            updateWorld(entity.get(), mat4::identity());
        }
    }
}

void Entities::updateWorld(Entity* entity, mat4 const& parentWorld)
{
    mat4 local = mat4::translate(entity->transform.position) *
                 mat4::rotateEuler(entity->transform.rotation);
    entity->worldMatrix = parentWorld * local;

    for (auto* child : entity->children)
    {
        updateWorld(child, entity->worldMatrix);
    }
}

entt::registry* Entities::registry() { return &m_registry; }