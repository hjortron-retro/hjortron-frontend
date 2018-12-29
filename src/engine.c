#include <SDL.h>
#include <SDL_events.h>
#include "engine.h"

extern scene_t main_scene;
extern scene_t run_game_scene;
extern scene_t game_menu_scene;

static void
_engine_handle_event(engine_t *engine, SDL_Event *event)
{
    scene_handle_event(engine->stack[engine->stack_idx], event);
}

static int
_engine_tick(engine_t *engine)
{
    return scene_tick(engine->stack[engine->stack_idx]);
}

static void
_engine_render(engine_t *engine)
{
    scene_render(engine->stack[engine->stack_idx]);
    SDL_RenderPresent(engine->renderer);
}

static void
_engine_scan_roms(engine_t *engine)
{
    scraper_scan_directory(&engine->scraper, &engine->cores,
        config_get(&engine->config, "/hjortron/directories/roms", "~/Games/emulation/"));
}

int
engine_init(engine_t *engine)
{
    int flags;
    memset(engine, 0, sizeof(engine_t));

    if (config_init(&engine->config) != 0)
      return 1;

    if (scraper_init(&engine->scraper) != 0)
      return 1;

    /* initialize collection of cores */
    if (core_collection_init(&engine->cores,
        config_get(&engine->config, "/hjortron/directories/cores", "/usr/lib/libretro")) != 0)
        return 1;

    /* scan rom directory and update datanase */
    _engine_scan_roms(engine);

    /* initialize scenes */
    main_scene.engine = engine;
    run_game_scene.engine = engine;
    game_menu_scene.engine = engine;

    /* mount main scene */
    if (main_scene.mount(&main_scene, NULL) != 0)
        return 1;

    engine->stack[0] = &main_scene;
    engine->stack_idx = 0;


    /* initialize SDL */
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMECONTROLLER);

    /* aux */
    TTF_Init();
    engine->font = TTF_OpenFont("/tmp/font.ttf", 18);

    /* video */
    flags = 0 /*|  SDL_WINDOW_FULLSCREEN_DESKTOP */;
    if (SDL_CreateWindowAndRenderer(320, 240, flags,
				    &engine->window, &engine->renderer) != 0)
      return 1;

    return 0;
}

void
engine_deinit(engine_t *engine)
{
    TTF_CloseFont(engine->font);
    TTF_Quit();

    SDL_DestroyRenderer(engine->renderer);
    SDL_DestroyWindow(engine->window);

    SDL_Quit();

    scraper_deinit(&engine->scraper);
    config_deinit(&engine->config);
}

static void
_engine_transform_keyboard_event_to_gamecontroller(SDL_Event *event)
{
    if (event->type != SDL_KEYDOWN)
        return;

    switch(event->key.keysym.scancode)
    {
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_RIGHT:
            event->type = SDL_CONTROLLERAXISMOTION;
            event->caxis.axis = SDL_CONTROLLER_AXIS_LEFTX;
            event->caxis.value = (event->key.keysym.scancode == SDL_SCANCODE_LEFT) ? -32000 : 32000;
        break;

        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_DOWN:
            event->type = SDL_CONTROLLERAXISMOTION;
            event->caxis.axis = SDL_CONTROLLER_AXIS_LEFTY;
            event->caxis.value = (event->key.keysym.scancode == SDL_SCANCODE_UP) ? -32000 : 32000;
        break;

        case SDL_SCANCODE_X: /* SDL_CONTROLLER_BUTTON_A */
        case SDL_SCANCODE_D: /* SDL_CONTROLLER_BUTTON_B */
        case SDL_SCANCODE_A: /* SDL_CONTROLLER_BUTTON_X */
        case SDL_SCANCODE_W: /* SDL_CONTROLLER_BUTTON_Y */
        case SDL_SCANCODE_ESCAPE: /* SDL_CONTROLLER_BUTTON_BACK */
            event->type = SDL_CONTROLLERBUTTONDOWN;
            event->cbutton.state = SDL_PRESSED;

            if(event->key.keysym.scancode == SDL_SCANCODE_X)
                event->cbutton.button = SDL_CONTROLLER_BUTTON_A;
            else if(event->key.keysym.scancode == SDL_SCANCODE_D)
                event->cbutton.button = SDL_CONTROLLER_BUTTON_B;
            else if(event->key.keysym.scancode == SDL_SCANCODE_A)
                event->cbutton.button = SDL_CONTROLLER_BUTTON_X;
            else if(event->key.keysym.scancode == SDL_SCANCODE_W)
                event->cbutton.button = SDL_CONTROLLER_BUTTON_Y;
            else if (event->key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                event->cbutton.button = SDL_CONTROLLER_BUTTON_BACK;

        break;
    }
}

int
engine_run(engine_t *engine)
{
    int quit;
    SDL_Event event;

    quit = 0;
    while(!quit)
    {
        while(SDL_PollEvent(&event))
        {
            /* Convert keyboard events to game controller events if bind */
            _engine_transform_keyboard_event_to_gamecontroller(&event);

            /* Dont push keyboard event down the chain */
            if (event.type != SDL_KEYDOWN || event.type != SDL_KEYUP)
                _engine_handle_event(engine, &event);

            if (event.type == SDL_QUIT)
                quit = 1;
        }
        if (_engine_tick(engine))
            _engine_render(engine);
    }

    return 0;
}

int
engine_push_scene(engine_t *engine, scene_t *scene, void *opaque)
{
    if (engine->stack_idx == (SCENE_STACK_SIZE - 1))
        return 1;

    if (scene->mount(scene, opaque) != 0)
        return 1;

    engine->stack_idx++;
    engine->stack[engine->stack_idx] = scene;

    return 0;
}

int
engine_pop_scene(engine_t *engine)
{
    fprintf(stderr, "Pop scene from stack of %d items\n", engine->stack_idx);
    /* prevent pop of first item on stack, the main menu scene */
    if (engine->stack_idx == 0)
        return 1;

    engine->stack_idx--;

    engine->stack[engine->stack_idx + 1]->unmount(engine->stack[engine->stack_idx + 1]);

    return 0;
}
