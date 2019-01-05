/*
 * This file is part of hjortron-frontend.
 *
 * Copyright 2019 Henrik Andersson <henrik.4e@gmail.com>
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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "scene.h"
#include "engine.h"
#include "transition.h"

typedef struct {
    bool dirty;
    uint32_t last_tick;
    uint32_t elapsed;
    transition_t transition;
    SDL_Texture *src;
    SDL_Texture *dest;
} transition_scene_data_t;

static int
_transition_scene_mount(struct scene_t *scene, void *opaque)
{
    transition_scene_data_t *data = scene->opaque;
    data->transition = *((transition_t *)opaque);
    return 0;
}

static void
_transition_scene_unmount(struct scene_t *scene)
{
}

static void
_transition_scene_enter(struct scene_t *scene)
{
    int w, h;
    transition_scene_data_t *data = scene->opaque;
    data->dirty = true;
    data->last_tick = 0;
    data->elapsed = 0;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);
    data->src = SDL_CreateTexture(scene->engine->renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    data->dest = SDL_CreateTexture(scene->engine->renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

    data->transition.source->enter(data->transition.source);

    /* mount and enter desitination scene */
    data->transition.dest->mount(data->transition.dest, data->transition.dest_opaque);
    data->transition.dest->enter(data->transition.dest);
}

static void
_transition_scene_leave(struct scene_t *scene)
{
    transition_scene_data_t *data = scene->opaque;

    data->transition.source->leave(data->transition.source);

    SDL_DestroyTexture(data->src);
    SDL_DestroyTexture(data->dest);
}

static void
_transition_scene_render_front(struct scene_t *scene, SDL_Renderer *renderer)
{
}

static void
_transition_scene_render_back(struct scene_t *scene, SDL_Renderer *renderer)
{
    transition_scene_data_t *data = scene->opaque;

    /* render transition scenes into textures*/
    SDL_SetRenderTarget(renderer, data->src);
    data->transition.source->render_back(data->transition.source, renderer);
    data->transition.source->render_front(data->transition.source, renderer);
    data->transition.source->render_overlay(data->transition.source, renderer);
    SDL_RenderPresent(renderer);

    SDL_SetRenderTarget(renderer, data->dest);
    data->transition.dest->render_back(data->transition.dest, renderer);
    data->transition.dest->render_front(data->transition.dest, renderer);
    data->transition.dest->render_overlay(data->transition.source, renderer);
    SDL_RenderPresent(renderer);

    /* render scenes into window */
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetTextureBlendMode(data->src, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(renderer, data->src, NULL, NULL);

    SDL_SetTextureBlendMode(data->dest, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(renderer, data->dest, NULL, NULL);


}

static void
_transition_scene_render_overlay(struct scene_t *scene, SDL_Renderer *renderer)
{
}

static int
_transition_scene_tick(struct scene_t *scene)
{
    uint32_t current_tick;
    transition_scene_data_t *data = scene->opaque;

    data->transition.source->tick(data->transition.source);
    data->transition.dest->tick(data->transition.dest);

    if (data->last_tick == 0)
    {
        data->last_tick = SDL_GetTicks();
    }

    /* Swap out transition scene with destination scene when duration
       has been reached */
    current_tick = SDL_GetTicks();
    if (current_tick >= data->last_tick + data->transition.duration)
    {
        /* pop the transition scene and push destination scene */
        engine_pop_scene(scene->engine);
        engine_push_scene(scene->engine,
                          data->transition.dest, data->transition.dest_opaque);
    }
    else
    {
        /* Tick the tranistion */
        switch(data->transition.type)
        {
            case TRANSITION_XFADE:
            {
                double amount = (current_tick - data->last_tick) / (double)data->transition.duration;
                SDL_SetTextureAlphaMod(data->src, 0xff * (1.0 - amount));
                SDL_SetTextureAlphaMod(data->dest, 0xff * amount);
            } break;

            case TRANSITION_ONLY_SRC:
            {
                SDL_SetTextureAlphaMod(data->src, 0xff);
                SDL_SetTextureAlphaMod(data->dest, 0x00);
            } break;
        }

    }



    return 1;
}

static void
_transition_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
}

transition_scene_data_t transition_scene_data = {};

scene_t transition_scene = {
    _transition_scene_mount,
    _transition_scene_unmount,
    _transition_scene_enter,
    _transition_scene_leave,
    _transition_scene_render_back,
    _transition_scene_render_front,
    _transition_scene_render_overlay,
    _transition_scene_tick,
    _transition_scene_handle_event,
    NULL,
    &transition_scene_data
};