#include "scene.h"
#include "engine.h"
#include "draw.h"
#include <SDL_ttf.h>

#define MENU_ITEMS 7

typedef struct menu_item_t {
    uint8_t id;
    char label[32];
    int (*on_menu_item)(struct menu_item_t *item, struct scene_t *scene);
} menu_item_t;

enum {
    GAME_BACK,
    GAME_QUIT,
    GAME_RESTART,
    GAME_LOAD,
    GAME_SAVE,
};

static int
_game_menu_item_handler(menu_item_t *item, struct scene_t *scene)
{
    switch(item->id)
    {
        case GAME_BACK: /* Back to game */
            engine_pop_scene(scene->engine);
            break;

        case GAME_RESTART: /* Restart game */
            break;

        case GAME_SAVE: /* Save game */
            break;

        case GAME_LOAD: /* Load game */
            break;

        case GAME_QUIT: /* Quit game */
            engine_pop_scene(scene->engine);
            engine_pop_scene(scene->engine);
            break;

        default:
            return 1;
    }
    return 0;
}

static menu_item_t _game_menu[] = {
    {GAME_BACK, "Back to game", _game_menu_item_handler},
    {GAME_QUIT, "Quit game", _game_menu_item_handler},
    {GAME_RESTART, "Restart game", _game_menu_item_handler},
    {GAME_LOAD, "Load game", _game_menu_item_handler},
    {GAME_SAVE, "Save game", _game_menu_item_handler},
};

typedef struct {
    scraper_rom_entry_t *rom_entry;
    int32_t index;
    menu_item_t *menu;
    uint8_t menu_item_cnt;
} game_menu_scene_data_t;


static int
_game_menu_scene_mount(struct scene_t *scene, void *opaque)
{
    game_menu_scene_data_t *data = scene->opaque;
    data->rom_entry = opaque;
    data->index = 0;
    return 0;
}

static void
_game_menu_scene_unmount(struct scene_t *scene)
{
}

static void
_game_menu_scene_render_front(struct scene_t *scene)
{
    int w,h;
    int s,e;
    SDL_Rect d;
    SDL_Color black = {0, 0, 0};
    game_menu_scene_data_t *data = scene->opaque;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);

    d.x = d.y = 0;
    d.w = w;
    d.h = h / MENU_ITEMS;

    s = e = 0;
    s = 0;
    e = data->menu_item_cnt;

    for (int i=s; i < e; i++)
    {
        draw_text(scene->engine->renderer, scene->engine->font,
            black, ALIGN_CENTER, data->menu[i].label, &d);

        d.y += d.h;
    }
}

static void
_game_menu_scene_render_back(struct scene_t *scene)
{
    int w, h;
    SDL_Rect center;
    game_menu_scene_data_t *data = scene->opaque;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);
    SDL_SetRenderDrawColor(scene->engine->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(scene->engine->renderer);

    /* draw item cursor line highlight */
    center.x = 0;
    center.w = w;
    center.h = h/MENU_ITEMS;
    center.y = data->index * center.h;
    SDL_SetRenderDrawColor(scene->engine->renderer, 0x0, 0x0, 0x0, 0x20);
    SDL_SetRenderDrawBlendMode(scene->engine->renderer, SDL_BLENDMODE_BLEND);

    SDL_RenderFillRect(scene->engine->renderer, &center);
}

static void
_game_menu_scene_render_overlay(struct scene_t *scene)
{
}

static int
_game_menu_scene_tick(struct scene_t *scene)
{
    return 1;
}

static void
_game_menu_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
    game_menu_scene_data_t *data = scene->opaque;

    if (event->type == SDL_CONTROLLERBUTTONDOWN)
    {
        switch(event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_A:
                _game_menu_item_handler(&data->menu[data->index], scene);
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                data->index--;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                data->index++;
                break;
        }
    }
}

game_menu_scene_data_t _game_menu_scene_data = {
    NULL,
    0,
    _game_menu,
    sizeof(_game_menu) / sizeof(menu_item_t),
};

scene_t game_menu_scene = {
    _game_menu_scene_mount,
    _game_menu_scene_unmount,
    _game_menu_scene_render_back,
    _game_menu_scene_render_front,
    _game_menu_scene_render_overlay,
    _game_menu_scene_tick,
    _game_menu_scene_handle_event,
    NULL,
    &_game_menu_scene_data
};