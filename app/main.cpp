#include <chrono>
#include <iostream>
#include <thread>

#include <renderlib/runtime.h>
#include <renderlib/graphicsBackends/raylib.h>
#include <renderlib/primitives.h>
#include <renderlib/camera.h>

#include <Raylib.h>

using namespace renderlib;

class TestSystem : public System
{
public:
    Entity* cube;
    Entity* fps;

    void onUpdate(Entities* entities, real dt)
    {
        const float speed = 100.0f;  // units per second
        if (IsKeyDown(KEY_W)) { cube->transform.position[1] += speed * dt; }
        if (IsKeyDown(KEY_S)) { cube->transform.position[1] -= speed * dt; }
        if (IsKeyDown(KEY_A)) { cube->transform.position[0] -= speed * dt; }
        if (IsKeyDown(KEY_D)) { cube->transform.position[0] += speed * dt; }
        if (IsKeyPressed(KEY_SPACE))
        {
            cube->transform.position[2] =
                cube->transform.position[2] == 0 ? 2 : 0;
        }

        auto text = fps->getComponent<Text2D>();
        text->color = vec4i{255, 255, 255, 255};
        text->text = std::format("FPS {}", dt > 0 ? 1 / dt : 0);
        text->fontSize = 24;

        cube->parent->transform.rotation[2] += dt * 10;
    }
};

int main()
{
    auto backend = std::make_unique<RaylibGraphicsBackend>();

    entt::entity cameraEntity = entt::null;

    Runtime runtime(std::move(backend), {1280, 960}, "Hello Renderlib");

    auto test = runtime.addSystem<TestSystem>();

    runtime.modify([&](Entities* entities) {
        {
            auto cameraEntity = entities->add("Camera");
            auto camera = cameraEntity->addComponent<OrthographicCamera>();
            cameraEntity->addComponent<OrthographicPan>();
            cameraEntity->addComponent<OrthographicZoom>();
            cameraEntity->transform.position = {
                0, 0, 200};  // straight down +Z, head-on
            camera->target = {0, 0, 0};  // look at the origin
            camera->up = {0, 1, 0};  // Y up → XY plane is the screen
            camera->width = 300.0f;  // tall enough for a 100-unit cube
        }
        {
            auto box2 = entities->add()->addComponent<Box>();
            box2->size = vec3{100, 100, 0};
            box2->color = vec4i{0, 255, 0, 100};
            box2->entity->transform.position[2] = 1;

            test->cube = entities->add(box2->entity);
            auto box = test->cube->addComponent<Box>();
            box->size = vec3{100, 100, 0};
            box->color = vec4i{255, 0, 0, 100};
            box->entity->transform.position[2] = 2;

            test->fps = entities->add();
            test->fps->addComponent<Text2D>();
        }
    });

    // Movement now happens on the render thread (see customRenderer above),
    // so the main thread just waits for the window to close.
    while (runtime.running())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return 0;
}
