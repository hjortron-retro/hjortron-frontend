#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <SDL_ttf.h>

#include "scene.h"
#include "engine.h"
#include "draw.h"

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


typedef struct {
    core_t *core;
    int32_t index;
    menu_item_t *menu;
    uint8_t menu_item_cnt;
} in_game_menu_scene_data_t;

static int
_in_game_menu_save_game(struct scene_t *scene, const char *filename)
{
    int fd = -1;
    uint8_t *state;
    size_t size;
    in_game_menu_scene_data_t *data = scene->opaque;

    size = data->core->api.retro_serialize_size();

    state = malloc(size);
    if (!data->core->api.retro_serialize(state, size))
        goto fail;


    /* store serialize data to disk */
    fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0)
        goto fail;

    if (write(fd, state, size) != size)
        goto fail;

    close(fd);
    free(state);
    return 0;

fail:
    if (fd > 0)
        close(fd);

    free(state);
    return 1;
}

static int
_in_game_menu_load_game(struct scene_t *scene, const char *filename)
{
    int fd;
    uint8_t *state;
    off_t size;
    in_game_menu_scene_data_t *data = scene->opaque;

    data->core->api.retro_reset();

    fd = open(filename, O_RDONLY);
    if (fd < 0)
        goto fail;

    size = lseek(fd, 0, SEEK_END);
    state = malloc(size);

    lseek(fd, 0, SEEK_SET);
    if (read(fd, state, size) != size)
        goto fail;

    if (!data->core->api.retro_unserialize(state, size))
        goto fail;

    close(fd);
    free(state);
    return 0;
fail:
    if (fd > 0)
        close(fd);
    free(state);
    return 1;
}

static int
_in_game_menu_item_handler(menu_item_t *item, struct scene_t *scene)
{
    in_game_menu_scene_data_t *data = scene->opaque;

    switch(item->id)
    {
        case GAME_BACK: /* Back to game */
            engine_pop_scene(scene->engine);
            break;

        case GAME_RESTART: /* Restart game */
            data->core->api.retro_reset();
            engine_pop_scene(scene->engine);
            break;

        case GAME_SAVE: /* Save game */
            if (_in_game_menu_save_game(scene, "/tmp/state.rom") != 0)
                fprintf(stderr, "%s(), failed to save game\n", __func__);
            engine_pop_scene(scene->engine);
            break;

        case GAME_LOAD: /* Load game */
            if (_in_game_menu_load_game(scene, "/tmp/state.rom") != 0)
                fprintf(stderr, "%s(), failed to load game\n", __func__);
            engine_pop_scene(scene->engine);
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

static menu_item_t _in_game_menu[] = {
    {GAME_BACK, "Back to game", _in_game_menu_item_handler},
    {GAME_QUIT, "Quit game", _in_game_menu_item_handler},
    {GAME_RESTART, "Restart game", _in_game_menu_item_handler},
    {GAME_LOAD, "Load game", _in_game_menu_item_handler},
    {GAME_SAVE, "Save game", _in_game_menu_item_handler},
};

static int
_in_game_menu_scene_mount(struct scene_t *scene, void *opaque)
{
    in_game_menu_scene_data_t *data = scene->opaque;
    data->core = opaque;
    data->index = 0;
    return 0;
}

static void
_in_game_menu_scene_unmount(struct scene_t *scene)
{
}

static void
_in_game_menu_scene_render_front(struct scene_t *scene)
{
    int w,h;
    int s,e;
    SDL_Rect d;
    SDL_Color black = {0, 0, 0};
    in_game_menu_scene_data_t *data = scene->opaque;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);

    d.x = d.y = 0;
    d.w = w;
    d.h = h / MENU_ITEMS;

    s = e = 0;
    s = 0;
    e = data->menu_item_cnt;

    for (int i=s; i < e; i++)
    {
        draw_text(scene->engine->renderer, scene->engine->font, TTF_STYLE_NORMAL,
            black, ALIGN_CENTER, data->menu[i].label, &d);

        d.y += d.h;
    }
}

static void
_in_game_menu_scene_render_back(struct scene_t *scene)
{
    int w, h;
    SDL_Rect center;
    in_game_menu_scene_data_t *data = scene->opaque;

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
_in_game_menu_scene_render_overlay(struct scene_t *scene)
{
}

static int
_in_game_menu_scene_tick(struct scene_t *scene)
{
    return 1;
}

static void
_in_game_menu_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
    in_game_menu_scene_data_t *data = scene->opaque;

    if (event->type == SDL_CONTROLLERBUTTONDOWN)
    {
        switch(event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_A:
                _in_game_menu_item_handler(&data->menu[data->index], scene);
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

in_game_menu_scene_data_t _in_game_menu_scene_data = {
    NULL,
    0,
    _in_game_menu,
    sizeof(_in_game_menu) / sizeof(menu_item_t),
};

scene_t in_game_menu_scene = {
    _in_game_menu_scene_mount,
    _in_game_menu_scene_unmount,
    _in_game_menu_scene_render_back,
    _in_game_menu_scene_render_front,
    _in_game_menu_scene_render_overlay,
    _in_game_menu_scene_tick,
    _in_game_menu_scene_handle_event,
    NULL,
    &_in_game_menu_scene_data
};