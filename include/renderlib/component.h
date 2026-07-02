#pragma once
#include <type_traits>

namespace renderlib {

class Entity;

struct Component
{
    Entity* entity;
};

template <typename T>
concept IsComponent = std::is_base_of_v<Component, T>;

}  // namespace renderlib