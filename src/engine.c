/*
 * This file is part of hjortron-frontend.
 *
 * Copyright 2018-2019 Henrik Andersson <henrik.4e@gmail.com>
 *
 * hjortron-frontend is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * hjortron-frontend is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hjortron-frontend.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 */

#include <SDL.h>
#include <SDL_events.h>
#include "engine.h"

extern scene_t main_scene;
extern scene_t run_game_scene;
extern scene_t in_game_menu_scene;

logger_level_t g_log_level = LOG_NOTICE;

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
    notice("engine", "scanning directory %s for available roms",
        config_get(&engine->config, "/hjortron/directories/roms", "~/Games/emulation/"));
    scraper_scan_directory(&engine->scraper, &engine->cores,
        config_get(&engine->config, "/hjortron/directories/roms", "~/Games/emulation/"));
}

int
engine_init(engine_t *engine)
{
    int w, h;
    int flags;
    memset(engine, 0, sizeof(engine_t));

    if (config_init(&engine->config) != 0)
      return 1;

    if (scraper_init(&engine->scraper) != 0)
      return 1;

    /* initialize collection of cores */
    notice("engine", "scanning for availble libretro cores");
    if (core_collection_init(&engine->cores,
        config_get(&engine->config, "/hjortron/directories/cores", "/usr/lib/libretro")) != 0)
    {
        error("engine", "no libretro cores was found at path '%s'",
            config_get(&engine->config, "/hjortron/directories/cores", "/usr/lib/libretro"));
        return 1;
    }

    /* scan rom directory and update datanase */
    _engine_scan_roms(engine);

    /* initialize scenes */
    main_scene.engine = engine;
    run_game_scene.engine = engine;
    in_game_menu_scene.engine = engine;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMECONTROLLER) < 0)
    {
        error("engine", "SDL_Init() failed: %s", SDL_GetError());
        return 1;
    }

    /* video */
    flags = 0;
    if (strcmp("true", config_get(&engine->config, "/hjortron/video/fullscreen", "false")) == 0)
        flags |=  SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (SDL_CreateWindowAndRenderer(320, 240, flags,
				    &engine->window, &engine->renderer) != 0)
    {
      error("engine", "SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
      return 1;
    }

    /* aux */
    TTF_Init();
    SDL_GetRendererOutputSize(engine->renderer, &w, &h);
    engine->font = TTF_OpenFont("/tmp/font.ttf", w * 0.06);
    if (engine->font == NULL)
    {
        error("engine", "failed to load font");
        return 1;
    }

    /* mount main scene */
    if (main_scene.mount(&main_scene, NULL) != 0)
    {
        error("engine", "failed to mount main scene");
        return 1;
    }

    engine->stack[0] = &main_scene;
    engine->stack_idx = 0;

    main_scene.enter(&main_scene);

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
    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.repeat != 0)
            return;

        event->type = SDL_CONTROLLERBUTTONDOWN;
        event->cbutton.state = SDL_PRESSED;
    }
    else if (event->type == SDL_KEYUP)
    {
        event->type = SDL_CONTROLLERBUTTONUP;
        event->cbutton.state = SDL_RELEASED;
    }
    else
    {
        /* not an event to be transformed */
        return;
    }

    switch(event->key.keysym.scancode)
    {
        case SDL_SCANCODE_LEFT:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
            break;

        case SDL_SCANCODE_RIGHT:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
            break;

        case SDL_SCANCODE_UP:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_UP;
            break;
        case SDL_SCANCODE_DOWN:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
            break;

        case SDL_SCANCODE_X:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_A;
            break;
        case SDL_SCANCODE_D:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_B;
            break;
        case SDL_SCANCODE_A:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_X;
            break;
        case SDL_SCANCODE_W:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_Y;
            break;
        case SDL_SCANCODE_ESCAPE:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_BACK;
            break;
        case SDL_SCANCODE_S:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_GUIDE;
            break;
        case SDL_SCANCODE_SPACE:
            event->cbutton.button = SDL_CONTROLLER_BUTTON_START;
            break;
        case SDL_SCANCODE_Q:
            event->cbutton.button = RETRO_DEVICE_ID_JOYPAD_L;
            break;
        case SDL_SCANCODE_E:
            event->cbutton.button = RETRO_DEVICE_ID_JOYPAD_R;
            break;

        default:
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

    scene->enter(scene);

    engine->stack_idx++;
    engine->stack[engine->stack_idx] = scene;

    return 0;
}

int
engine_pop_scene(engine_t *engine)
{
    scene_t *scene;

    /* prevent pop of first item on stack, the main menu scene */
    if (engine->stack_idx == 0)
        return 1;

    scene = engine->stack[engine->stack_idx];
    engine->stack_idx--;

    scene->leave(scene);
    scene->unmount(scene);

    /* enter new scene */
    scene = engine->stack[engine->stack_idx];
    scene->enter(scene);

    return 0;
}
