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
#include <SDL.h>

extern scene_t transition_scene;

#include "logo.c"

typedef struct {
    bool dirty;
    SDL_Color color;
} blank_scene_data_t;


static int
_blank_scene_mount(struct scene_t *scene, void *opaque)
{
    SDL_Color *color;
    blank_scene_data_t *data = scene->opaque;
    color = (SDL_Color*)opaque;
    data->color = *color;
    return 0;
}

static void
_blank_scene_unmount(struct scene_t *scene)
{
}

static void
_blank_scene_enter(struct scene_t *scene)
{
    blank_scene_data_t *data = scene->opaque;
    data->dirty = true;
}

static void
_blank_scene_leave(struct scene_t *scene)
{
}

static void
_blank_scene_render_front(struct scene_t *scene, SDL_Renderer *renderer)
{
}

static void
_blank_scene_render_back(struct scene_t *scene, SDL_Renderer *renderer)
{
    blank_scene_data_t *data = scene->opaque;
    SDL_SetRenderDrawColor(renderer,
                           data->color.r, data->color.g, data->color.b, 0xff);
    SDL_RenderClear(renderer);
}

static void
_blank_scene_render_overlay(struct scene_t *scene, SDL_Renderer *renderer)
{
}

static int
_blank_scene_tick(struct scene_t *scene)
{
    blank_scene_data_t *data = scene->opaque;

    if (data->dirty)
    {
        data->dirty = false;
        return 1;
    }

    return 1;
}

static void
_blank_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
}

blank_scene_data_t blank_scene_data = {
    true,
    {0xff,0xff,0xff,0xff}
};

scene_t blank_scene = {
    _blank_scene_mount,
    _blank_scene_unmount,
    _blank_scene_enter,
    _blank_scene_leave,
    _blank_scene_render_back,
    _blank_scene_render_front,
    _blank_scene_render_overlay,
    _blank_scene_tick,
    _blank_scene_handle_event,
    NULL,
    &blank_scene_data
};