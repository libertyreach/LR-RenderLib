cmake_minimum_required(VERSION 3.25)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use: cmake -B build")
endif()

project(renderlib VERSION 1.0.0 LANGUAGES CXX)

# --- Standard --------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # -std=c++20, not -std=gnu++20

# Emit build/compile_commands.json so clangd (editor IntelliSense) knows the
# include paths and flags for each file.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Debug for single-config generators (Ninja/Make) if unspecified.
if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Put binaries in build/ when this is the top-level project
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# --- Third-party dependencies ----------------------------------------------
# Both are vendored as full source checkouts under thirdparty/, so we just
# add_subdirectory them and link their targets. No find_package / install needed.

# EnTT is header-only. Its own CMakeLists requires CMake >= 3.28 (we're on
# 3.26), so instead of add_subdirectory we wrap the amalgamated single header
# in an INTERFACE target. Same EnTT::EnTT name, so consumers/link lines match.
add_library(EnTT INTERFACE)
add_library(EnTT::EnTT ALIAS EnTT)
target_include_directories(EnTT INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/entt/single_include)
target_compile_features(EnTT INTERFACE cxx_std_20)

# magic_enum is header-only. We wrap it in an INTERFACE target (same style as
# EnTT above) rather than add_subdirectory, keeping the same magic_enum::magic_enum
# name its own CMake exports so consumers/link lines match. Headers live under
# include/magic_enum/, so includes are #include <magic_enum/magic_enum.hpp>.
add_library(magic_enum INTERFACE)
add_library(magic_enum::magic_enum ALIAS magic_enum)
target_include_directories(magic_enum INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/magic_enum/include)
target_compile_features(magic_enum INTERFACE cxx_std_20)

# raylib: stop it from building its examples and game samples.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

# EXCLUDE_FROM_ALL: only build raylib if renderlib actually uses it.
add_subdirectory(thirdparty/raylib EXCLUDE_FROM_ALL)

# --- Library target --------------------------------------------------------
# CONFIGURE_DEPENDS re-globs on rebuild so new .cpp files are picked up
# without a manual re-configure.
file(GLOB_RECURSE RENDERLIB_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(renderlib STATIC ${RENDERLIB_SOURCES})
add_library(renderlib::renderlib ALIAS renderlib)  # consumers use the namespaced name

# PUBLIC because a rendering/ECS library typically exposes entt + raylib types
# in its own public headers (include/renderlib/*). If a dependency ends up used
# ONLY inside src/*.cpp, change it to PRIVATE so it doesn't leak to consumers.
target_link_libraries(renderlib PUBLIC
    EnTT::EnTT
    magic_enum::magic_enum
    raylib
)

# PUBLIC: consumers (and the lib itself) get include/ on their include path.
target_include_directories(renderlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(renderlib PUBLIC cxx_std_20)

target_compile_options(renderlib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# --- Demo / test executable ------------------------------------------------
# Only built when renderlib is the main project, not when it's pulled in
# as a subdirectory by another project.
if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(app)
endif()
