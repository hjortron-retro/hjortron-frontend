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

#ifndef _draw_h
#define _draw_h

#include <SDL.h>
#include <SDL_ttf.h>

typedef enum draw_text_align_t
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} draw_text_align_t;

void draw_text(SDL_Renderer *renderer,
                TTF_Font *font, int style, SDL_Color color,
                draw_text_align_t align,
                const char *text, SDL_Rect *dest);

SDL_Surface *render_text(TTF_Font *font, int style, SDL_Color color, const char *text);


#endif /* draw */