# renderlib

A small C++20 rendering engine built around an **entity-component-system** (ECS)
core with a **pluggable graphics backend**. The engine itself knows nothing about
any specific rendering library — it describes a scene as entities and components,
and a backend is responsible for turning that into pixels. A [raylib](https://www.raylib.com/)
backend ships in the box.

> Status: early / in active development. The API is small and still moving.

## Features

- **ECS built on [EnTT](https://github.com/skypjack/entt)** — compose entities from
  components (`Box`, `Sphere`, `Text2D`, `Text3D`, cameras, …).
- **Systems with a lifecycle** — `onBeforeUpdate` / `onUpdate(dt)` / `onAfterUpdate`.
- **Hierarchical transforms** — parent/child relationships compose full TRS
  transforms (translation + rotation carry down the tree).
- **Cameras** — orthographic and perspective, plus mouse **pan** (drag) and
  **zoom-to-cursor** (wheel) controls as opt-in components.
- **Transparency handling** — translucent primitives are depth-sorted back-to-front.
- **Backend abstraction** — implement `itf::GraphicsBackend` to target something
  other than raylib.

## Requirements

- A **C++20** compiler (Clang recommended; MSVC and GCC should also work)
- **CMake ≥ 3.25**
- **Ninja** (used by the build script for `compile_commands.json` / editor tooling)
- **Git** (dependencies are vendored as submodules)

Dependencies ([EnTT](https://github.com/skypjack/entt),
[raylib](https://github.com/raysan5/raylib),
[magic_enum](https://github.com/Neargye/magic_enum)) are pulled in as git
submodules under `thirdparty/` — there is nothing to install system-wide.

## Getting the source

Clone with submodules:

```sh
git clone --recurse-submodules <repo-url>
cd renderlib
```

Already cloned without `--recurse-submodules`? Fetch them:

```sh
git submodule update --init --recursive
```

## Building

### With the build script (Windows / PowerShell)

```powershell
./build.ps1
```

This configures with Ninja + Clang in **Release** and builds both the library
and the demo. The demo lands at `build/bin/renderlib_demo`.

### Manually (any platform)

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Outputs:

- `build/lib/renderlib.lib` (or `librenderlib.a`) — the static library
- `build/bin/renderlib_demo` — the demo executable (built only when renderlib is
  the top-level project)

## Running the demo

```sh
./build/bin/renderlib_demo
```

The demo (`app/main.cpp`) opens a window with two translucent squares, moves one
with **WASD** (Space toggles its depth), and shows an FPS readout. Middle-drag to
pan the camera and scroll to zoom.

## Usage

### A minimal app

```cpp
#include <chrono>
#include <thread>

#include <renderlib/runtime.h>
#include <renderlib/graphicsBackends/raylib.h>
#include <renderlib/primitives.h>
#include <renderlib/camera.h>

using namespace renderlib;

int main()
{
    // A Runtime owns a backend, opens a window, and drives the render loop
    // on its own thread.
    Runtime runtime(
        std::make_unique<RaylibGraphicsBackend>(), {1280, 960}, "My App");

    // Populate the world. modify() takes a lock, so it is safe to call while
    // the render thread is running.
    runtime.modify([](Entities* entities) {
        // Orthographic camera looking down -Z at the origin, +Y up.
        auto cameraEntity = entities->add("Camera");
        auto camera = cameraEntity->addComponent<OrthographicCamera>();
        cameraEntity->addComponent<OrthographicPan>();   // middle-drag to pan
        cameraEntity->addComponent<OrthographicZoom>();  // wheel to zoom
        cameraEntity->transform.position = {0, 0, 200};
        camera->target = {0, 0, 0};
        camera->width  = 300.0f;  // visible height, in world units

        // A red square (a flat box).
        auto entity = entities->add("Box");
        auto box = entity->addComponent<Box>();
        box->size  = {100, 100, 0};
        box->color = {255, 0, 0, 255};  // RGBA, 0–255
    });

    // The render loop runs on its own thread; the main thread just waits for
    // the window to close.
    while (runtime.running())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return 0;
}
```

### Entities and components

Entities are created through `Entities` and given behavior by attaching
components:

```cpp
auto entity = entities->add("player");
entity->transform.position = {10, 0, 0};  // local transform
entity->transform.rotation = {0, 0, 45};  // Euler angles in degrees

auto box = entity->addComponent<Box>();
box->size  = {50, 50, 0};
box->color = {0, 128, 255, 255};

// Look components up later:
if (auto* b = entity->getComponent<Box>()) { b->color = {255, 255, 255, 255}; }
```

Built-in primitive components (`renderlib/primitives.h`):

| Component | Fields |
|-----------|--------|
| `Box`     | `vec4i color`, `vec3 size` |
| `Sphere`  | `vec4i color`, `float radius` |
| `Text2D`  | `vec4i color`, `vec2 position`, `string text`, `int fontSize` (screen space) |
| `Text3D`  | `vec4i color`, `string text`, `int fontSize` (positioned in the world) |

Colors are `vec4i` RGBA in the 0–255 range. An alpha below 255 marks the
primitive as translucent and routes it through the back-to-front sort.

### Systems

Systems hold per-frame logic. Subclass `System`, override the hooks you need,
and register the system with the runtime:

```cpp
#include <raylib.h>  // systems run on the render thread, so raylib input is available

class Mover : public System
{
public:
    Entity* target = nullptr;

    void onUpdate(Entities* entities, real dt) override
    {
        const float speed = 100.0f;  // units per second
        if (IsKeyDown(KEY_D)) target->transform.position[0] += speed * dt;
        if (IsKeyDown(KEY_A)) target->transform.position[0] -= speed * dt;
    }
};

auto mover = runtime.addSystem<Mover>();
// ... set mover->target to an entity you created via modify() ...
```

Each frame the runtime runs `onBeforeUpdate` → `onUpdate(dt)` → `onAfterUpdate`
for every system, recomputes world transforms, then renders.

### Cameras and controls

Attach a camera component to an entity; its `transform.position` is the camera
position.

- `OrthographicCamera` — `target`, `up`, and `width` (the visible height in world units)
- `PerspectiveCamera` — `target`, `up`, and `fov`
- `OrthographicPan` — middle-mouse drag pans the view (`speed` = drag sensitivity, `1.0` tracks the cursor exactly)
- `OrthographicZoom` — mouse wheel zooms toward the cursor (`speed` = fraction zoomed per tick)

### Hierarchy

Add an entity as a child of another to make its transform relative to the parent.
Moving or rotating the parent carries the children with it:

```cpp
auto planet = entities->add("planet");
planet->addComponent<Box>()->size = {80, 80, 0};

auto moon = entities->add(planet, "moon");  // child of planet
moon->transform.position = {150, 0, 0};      // offset from the planet
moon->addComponent<Box>()->size = {20, 20, 0};

// Later, in a system: rotating the planet orbits the moon.
planet->transform.rotation[2] += 30.0f * dt;  // degrees/sec about Z
```

## Using renderlib in your own project

renderlib is CMake-friendly. Add it as a subdirectory and link the namespaced
target — the demo executable builds only when renderlib is the top-level project,
so it won't get in your way:

```cmake
add_subdirectory(renderlib)
target_link_libraries(your_app PRIVATE renderlib::renderlib)
```

Public headers live under `include/renderlib/` and are included as
`#include <renderlib/...>`.

## Project layout

```
include/renderlib/   public headers (engine API)
  itf/               backend interface (GraphicsBackend)
  graphicsBackends/  concrete backends (raylib)
src/                 library implementation
app/                 demo executable (main.cpp)
thirdparty/          vendored submodules (entt, raylib, magic_enum)
CMakeLists.txt       build configuration
build.ps1            convenience build script (Windows)
```
