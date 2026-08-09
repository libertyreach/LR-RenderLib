#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <string_view>
#include <optional>
#include <array>
#include "math.h"


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

using vec4i = Vector<int, 4>;
using vec3i = Vector<int, 3>;
using vec2i = Vector<int, 2>;
using vec4 = Vector<real, 4>;
using vec3 = Vector<real, 3>;
using vec2 = Vector<real, 2>;
using mat4 = Mat4<real>;

}  // namespace renderlib