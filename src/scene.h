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

#ifndef _scene_h
#define _scene_h

#include <SDL.h>

typedef struct scene_t {
    int (*mount)(struct scene_t *scene, void *opaque);
    void (*unmount)(struct scene_t *scene);
    void (*enter)(struct scene_t *scene);
    void (*leave)(struct scene_t *scene);
    void (*render_back)(struct scene_t *scene, SDL_Renderer *renderer);
    void (*render_front)(struct scene_t *scene, SDL_Renderer *renderer);
    void (*render_overlay)(struct scene_t *scene, SDL_Renderer *renderer);
    int (*tick)(struct scene_t *scene);
    void (*handle_event)(struct scene_t *scene, SDL_Event *event);
    struct engine_t *engine;
    void *opaque;
} scene_t;

void scene_render(scene_t *scene, SDL_Renderer *renderer);
void scene_handle_event(scene_t *scene, SDL_Event *event);
int scene_tick(scene_t *scene);

#endif /* _scene_h */