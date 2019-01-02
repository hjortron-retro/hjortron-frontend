#ifndef _engine_h
#define _engine_h

#include <SDL_ttf.h>

#include "logger.h"
#include "scraper.h"
#include "config.h"
#include "core.h"
#include "scene.h"

#define SCENE_STACK_SIZE 5

typedef struct engine_t
{
    config_t config;
    scraper_t scraper;

    core_collection_t cores;

    TTF_Font *font;
    SDL_Window *window;
    SDL_Renderer *renderer;

    scene_t *stack[SCENE_STACK_SIZE];
    int stack_idx;

} engine_t;

int engine_init(engine_t *engine);
void engine_deinit(engine_t *engine);
int engine_run(engine_t *engine);
int engine_push_scene(engine_t *engine, scene_t *scene, void *opaque);
int engine_pop_scene(engine_t *engine);

#endif /* _engine_h */
