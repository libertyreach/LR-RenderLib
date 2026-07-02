#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <string_view>
#include <optional>
#include <array>


namespace renderlib {

using string = std::string;

using string_view = std::string_view;

template <typename T>
using list = std::vector<T>;

template <typename K, typename V>
using map = std::unordered_map<K, V>;

template <typename T>
using set = std::set<T>;

template <typename T>
using optional = std::optional<T>;

template <typename T>
using interval = std::tuple<T, T>;

using real = float;

using vec4i = std::array<int, 4>;
using vec3i = std::array<int, 3>;
using vec2i = std::array<int, 2>;
using vec4 = std::array<real, 4>;
using vec3 = std::array<real, 3>;
using vec2 = std::array<real, 2>;

}  // namespace renderlib