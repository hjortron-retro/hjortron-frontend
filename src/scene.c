#include "scene.h"

void scene_render(scene_t *scene)
{
    if (scene->render_back)
        scene->render_back(scene);
    if (scene->render_front)
        scene->render_front(scene);
    if (scene->render_overlay)
        scene->render_overlay(scene);
}

void scene_handle_event(scene_t *scene, SDL_Event *event)
{
    if (scene->handle_event)
        scene->handle_event(scene, event);
}

int scene_tick(scene_t *scene)
{
    if (scene->tick)
        return scene->tick(scene);

    return 0;
}