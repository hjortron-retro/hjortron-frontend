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

#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "scene.h"
#include "engine.h"
#include "draw.h"


static int MENU_BAR_ITEM_SIZE = 0;
#define ROM_ENTRIES 8

extern scene_t run_game_scene;
static char *menu_bar_items = "abcdefghijklmnopqrstuvwxyz";


typedef struct {
    int menu;

    struct {
        scraper_rom_entry_t entries[ROM_ENTRIES];
        size_t entry_cnt;
        int32_t offset;
        int32_t index;
    } menu_roms;

    struct {
        SDL_Texture *texture;
        int32_t index;
    } menu_bar;

    bool dirty;

} main_scene_data_t;

static int
_main_scene_update_rom_entries(scene_t *scene)
{
    int i;
    main_scene_data_t *data = scene->opaque;

    /* free up entries */
    for (i = 0; i < ROM_ENTRIES; i++)
    {
        free((void *)data->menu_roms.entries[i].name);
        free((void *)data->menu_roms.entries[i].path);
        free((void *)data->menu_roms.entries[i].core);
    }

    memset(&data->menu_roms.entries, 0, sizeof(data->menu_roms.entries));
    data->menu_roms.entry_cnt = ROM_ENTRIES;

    /* get new entries */
    if (scraper_get_list(&scene->engine->scraper, data->menu_roms.offset, ROM_ENTRIES,
        data->menu_roms.entries, &data->menu_roms.entry_cnt) != 0)
        data->menu_roms.entry_cnt = 0;

    return 0;
}

static void
_main_scene_side_menu_bar_texture(struct scene_t *scene)
{
    char c[2] = {0}, *pitem;
    SDL_Rect dest;
    SDL_Surface *ch, *menu_bar;
    SDL_Color black = {220, 220, 220, 0x2f};
    main_scene_data_t *data = scene->opaque;

    menu_bar = SDL_CreateRGBSurfaceWithFormat(0, MENU_BAR_ITEM_SIZE,
                                              MENU_BAR_ITEM_SIZE * (27 + 1),
                                              32, SDL_PIXELFORMAT_RGBA32);

    dest.x = dest.y = 0;
    pitem = menu_bar_items;
    while(*pitem != '\0')
    {
        *c = *pitem;
        ch = render_text(scene->engine->font, TTF_STYLE_BOLD, black, c);
        SDL_SetSurfaceBlendMode(ch, SDL_BLENDMODE_NONE);
        if (ch)
        {
            dest.x = (menu_bar->w / 2) - (ch->w / 2);
            dest.w = ch->w;
            dest.h = ch->h;

            SDL_BlitSurface(ch, NULL, menu_bar, &dest);
            SDL_FreeSurface(ch);
        }

        dest.y += MENU_BAR_ITEM_SIZE;
        pitem++;
    }

    data->menu_bar.texture = SDL_CreateTextureFromSurface(scene->engine->renderer, menu_bar);
    SDL_SetTextureBlendMode(data->menu_bar.texture, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(menu_bar);
}

static int
_main_scene_mount(struct scene_t *scene, void *opaque)
{
    int w,h;
    main_scene_data_t *data = scene->opaque;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);
    MENU_BAR_ITEM_SIZE = w*0.075;

    /* create side menu bar texture */
    _main_scene_side_menu_bar_texture(scene);

    /* fetch first set of available roms */
    data->menu_roms.index = data->menu_roms.offset = 0;
    _main_scene_update_rom_entries(scene);

    return 0;
}

static void
_main_scene_unmount(struct scene_t *scene)
{
    main_scene_data_t *data = scene->opaque;
    if (data->menu_bar.texture)
        SDL_DestroyTexture(data->menu_bar.texture);
}

static void
_main_scene_enter(struct scene_t *scene)
{
    main_scene_data_t *data = scene->opaque;
    data->dirty = true;
}

static void
_main_scene_leave(struct scene_t *scene)
{
}

static void
_main_scene_render_front(struct scene_t *scene)
{
    int i;
    int w, h;
    SDL_Rect d;
    main_scene_data_t *data = scene->opaque;
    SDL_Color black = {0, 0, 0};

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);

    d.x = d.y = 0;
    d.x = MENU_BAR_ITEM_SIZE + 4;
    d.w = w;
    d.h = h / ROM_ENTRIES;
    for (i = 0; i < data->menu_roms.entry_cnt; i++)
    {
        draw_text(scene->engine->renderer, scene->engine->font,
                  TTF_STYLE_NORMAL, black, ALIGN_LEFT, data->menu_roms.entries[i].name, &d);

        d.y += d.h;
    }

}

static void
_main_scene_render_back(struct scene_t *scene)
{
    uint32_t fmt;
    int w, h, sw, sh, access;
    SDL_Rect center;
    main_scene_data_t *data = scene->opaque;

    /*clear screen */
    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);
    SDL_SetRenderDrawColor(scene->engine->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(scene->engine->renderer);

    /* draw side menu bar */
    center.x = center.y = 0;
    center.w = MENU_BAR_ITEM_SIZE;
    center.h = h;
    SDL_SetRenderDrawColor(scene->engine->renderer, 0x0, 0x0, 0x0, 0xa0);
    SDL_SetRenderDrawBlendMode(scene->engine->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(scene->engine->renderer, &center);

    /* draw item cursor line highlight */
    if (data->menu == 0)
    {
        center.x = MENU_BAR_ITEM_SIZE;
        center.w = w;
        center.h = h/ROM_ENTRIES;
        center.y = data->menu_roms.index * center.h;
        SDL_SetRenderDrawColor(scene->engine->renderer, 0x0, 0x0, 0x0, 0x20);
        SDL_SetRenderDrawBlendMode(scene->engine->renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(scene->engine->renderer, &center);
    }

    center.x = center.y = 0;
    center.w = center.h = MENU_BAR_ITEM_SIZE;
    center.y = (h/2) - (MENU_BAR_ITEM_SIZE/2);
    SDL_SetRenderDrawColor(scene->engine->renderer,
                           0x0, 0x0, 0x0,
                           data->menu == 0 ? 0x40 : 0x7f);
    SDL_SetRenderDrawBlendMode(scene->engine->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(scene->engine->renderer, &center);

    /* render menu_bar */
    SDL_QueryTexture(data->menu_bar.texture, &fmt, &access, &sw, &sh);
    center.x = center.y = 0;
    center.y = (h/2) - (MENU_BAR_ITEM_SIZE/2);
    center.w = sw;
    center.h = sh;
    center.y -= (data->menu_bar.index * MENU_BAR_ITEM_SIZE);
    SDL_RenderCopy(scene->engine->renderer, data->menu_bar.texture, NULL, &center);
}

static void
_main_scene_render_overlay(struct scene_t *scene)
{
}

static int
_main_scene_tick(struct scene_t *scene)
{
    main_scene_data_t *data = scene->opaque;

    if (data->dirty == false)
    {
        SDL_Delay(25);
        return 0;
    }

    data->dirty = false;
    return 1;
}

static void
_main_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
    char rom_ch;
    main_scene_data_t *data = scene->opaque;

    if (event->type == SDL_CONTROLLERBUTTONDOWN)
    {
        switch (event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                if (data->menu == 0)
                {
                    data->menu_roms.index--;
                    if (data->menu_roms.index < 0)
                    {
                        data->menu_roms.offset--;
                        data->menu_roms.index = 0;
                        _main_scene_update_rom_entries(scene);
                    }

                    rom_ch = tolower(data->menu_roms.entries[data->menu_roms.index].name[0]);
                    data->menu_bar.index = rom_ch - 'a';
                }
                else
                {
                    if (data->menu_bar.index > 0)
                        data->menu_bar.index--;
                }
                data->dirty = true;
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                if (data->menu == 0)
                {
                    data->menu_roms.index++;
                    if (data->menu_roms.index >= ROM_ENTRIES)
                    {
                        data->menu_roms.offset++;
                        data->menu_roms.index = ROM_ENTRIES - 1;
                        _main_scene_update_rom_entries(scene);
                    }

                    rom_ch = tolower(data->menu_roms.entries[data->menu_roms.index].name[0]);
                    data->menu_bar.index = rom_ch - 'a';
                }
                else
                {
                    if (data->menu_bar.index < 25)
                        data->menu_bar.index++;
                }
                data->dirty = true;
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                data->menu = 1; /* menu bar */
                data->dirty = true;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                data->menu = 0; /* game list */
                data->dirty = true;
                break;

            case SDL_CONTROLLER_BUTTON_A:
                if (data->menu == 0)
                {
                    /* start selected game */
                    if (engine_push_scene(scene->engine, &run_game_scene,
                                    &data->menu_roms.entries[data->menu_roms.index]) != 0)
                    {
                        warning("main_scene", "failed to start game");
                    }
                }
                else
                {
                    /* jump to char */
                    data->menu_roms.offset =
                        scraper_get_offset_to_char(&scene->engine->scraper, menu_bar_items[data->menu_bar.index]);
                    _main_scene_update_rom_entries(scene);
                    data->menu_roms.index = 0;
                }
                data->dirty = true;
                break;

            case SDL_CONTROLLER_BUTTON_BACK:
                {
                    SDL_QuitEvent e;
                    e.type = SDL_QUIT;
                    e.timestamp = time(NULL);
                    SDL_PushEvent((SDL_Event*)&e);
                }
                break;

        }
    }
}

main_scene_data_t _main_scene_data = {
    0
};

scene_t main_scene = {
    _main_scene_mount,
    _main_scene_unmount,
    _main_scene_enter,
    _main_scene_leave,
    _main_scene_render_back,
    _main_scene_render_front,
    _main_scene_render_overlay,
    _main_scene_tick,
    _main_scene_handle_event,
    NULL,
    &_main_scene_data
};