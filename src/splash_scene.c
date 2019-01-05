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

extern scene_t main_scene;
extern scene_t transition_scene;

#include "logo.c"

typedef struct {
    bool dirty;
    uint32_t last_time;
    SDL_Texture *logo;
    SDL_Rect logo_size;
} splash_scene_data_t;


static int
_splash_scene_mount(struct scene_t *scene, void *opaque)
{
    return 0;
}

static void
_splash_scene_unmount(struct scene_t *scene)
{
}

static void
_splash_scene_enter(struct scene_t *scene)
{
    SDL_Surface *surface;
    splash_scene_data_t *data = scene->opaque;

    surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)logo.pixel_data,
        logo.width, logo.height,
        32, logo.width * 4,
        SDL_PIXELFORMAT_ABGR8888);
    data->logo_size.x = data->logo_size.y = 0;
    data->logo_size.w = surface->w;
    data->logo_size.h = surface->h;
    data->logo = SDL_CreateTextureFromSurface(scene->engine->renderer, surface);
    SDL_FreeSurface(surface);
    data->last_time = 0;
    data->dirty = true;
}

static void
_splash_scene_leave(struct scene_t *scene)
{
    splash_scene_data_t *data = scene->opaque;
    if (data->logo)
    {
        SDL_DestroyTexture(data->logo);
        data->logo = NULL;
    }
}

static void
_splash_scene_render_front(struct scene_t *scene, SDL_Renderer *renderer)
{
    int w,h;
    SDL_Rect d;
    splash_scene_data_t *data = scene->opaque;

    /* Draw logo */
    d = data->logo_size;
    if (data->logo)
    {
        SDL_GetRendererOutputSize(renderer, &w, &h);
        d.x = (w / 2) - (d.w / 2);
        d.y = (h / 2) - (d.h / 2);
        SDL_RenderCopy(renderer, data->logo, NULL, &d);
    }
}

static void
_splash_scene_render_back(struct scene_t *scene, SDL_Renderer *renderer)
{
    int w, h;

    /* clear scene */
    SDL_GetRendererOutputSize(renderer, &w, &h);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(renderer);
}

static void
_splash_scene_render_overlay(struct scene_t *scene, SDL_Renderer *renderer)
{
}

static int
_splash_scene_tick(struct scene_t *scene)
{
    transition_t transition;
    splash_scene_data_t *data = scene->opaque;

    if (data->last_time == 0)
    {
        data->last_time = SDL_GetTicks();
    }

    if (SDL_GetTicks() > (data->last_time + 1000))
    {
        transition.type = TRANSITION_XFADE;
        transition.duration = 500;
        transition.source = scene;
        transition.source_opaque = NULL;
        transition.dest = &main_scene;
        transition.dest_opaque = NULL;
        engine_push_scene(scene->engine, &transition_scene, &transition);
        return 0;
    }

    if (data->dirty)
    {
        data->dirty = false;
        return 1;
    }

    return 0;
}

static void
_splash_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
}

splash_scene_data_t splash_scene_data = {};

scene_t splash_scene = {
    _splash_scene_mount,
    _splash_scene_unmount,
    _splash_scene_enter,
    _splash_scene_leave,
    _splash_scene_render_back,
    _splash_scene_render_front,
    _splash_scene_render_overlay,
    _splash_scene_tick,
    _splash_scene_handle_event,
    NULL,
    &splash_scene_data
};