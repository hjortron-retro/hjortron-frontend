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