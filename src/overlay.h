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

#ifndef _overlay_h
#define _overlay_h

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>

enum {
    ICON_BATTERY_0 = 0,
    ICON_BATTERY_1,
    ICON_BATTERY_2,
    ICON_BATTERY_3,
    ICON_BATTERY_4,

    ICON_LIGHT_BULB,

    ICON_SPEAKER_MUTE,
    ICON_SPEAKER_LOW,
    ICON_SPEAKER_HIGH,

    ICON_CNT
};

typedef struct overlay_t {
    struct engine_t *engine;
    bool show;
    bool dirty;
    uint32_t button_press_tick;
    uint32_t button_release_tick;

    int volume;
    int brightness;

    uint32_t size;
    TTF_Font *font;
    SDL_Texture *icons[ICON_CNT];

} overlay_t;

int overlay_init(overlay_t *overlay, SDL_Renderer *renderer, uint32_t size);
void overlay_render(overlay_t *overlay, SDL_Renderer *renderer);
int overlay_handle_event(overlay_t *overlay, SDL_Event *event);
int overlay_tick(overlay_t *overlay);

#endif /* _overlay_h */