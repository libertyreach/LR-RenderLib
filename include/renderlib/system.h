#pragma once

#include <type_traits>
#include <entt/entt.hpp>
#include "entities.h"
#include "types.h"

namespace renderlib {

struct System
{
    virtual ~System() = default;

    virtual void onBeforeUpdate(Entities* entities) {};
    virtual void onUpdate(Entities* entities, real dt) {};
    virtual void onAfterUpdate(Entities* entities) {};
};

template <typename T>
concept IsSystem = std::is_base_of_v<System, T>;

}  // namespace renderlib