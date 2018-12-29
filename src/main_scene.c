#include "scene.h"
#include "engine.h"
#include "draw.h"
#include <SDL_ttf.h>

#define ROM_ENTRIES 7

SDL_Color black = {0, 0, 0};

extern scene_t run_game_scene;

typedef struct {
    scraper_rom_entry_t entries[ROM_ENTRIES];
    size_t entry_cnt;
    uint32_t offset;
    uint32_t index;
} main_scene_data_t;

static int
_main_scene_update_rom_entries(scene_t *scene)
{
    int i;
    main_scene_data_t *data = scene->opaque;

    /* free up entries */
    for (i = 0; i < ROM_ENTRIES; i++)
    {
        free((void *)data->entries[i].name);
        free((void *)data->entries[i].path);
        free((void *)data->entries[i].core);
    }

    memset(&data->entries, 0, sizeof(data->entries));
    data->entry_cnt = ROM_ENTRIES;

    /* get new entries */
    if (scraper_get_list(&scene->engine->scraper, data->offset, ROM_ENTRIES, data->entries, &data->entry_cnt) != 0)
        data->entry_cnt = 0;

    return 0;
}

static int
_main_scene_mount(struct scene_t *scene, void *opaque)
{
    main_scene_data_t *data = scene->opaque;

    /* fetch first set of available roms */
    data->offset = 0;
    _main_scene_update_rom_entries(scene);

    return 0;
}

static void
_main_scene_unmount(struct scene_t *scene)
{

}

static void
_main_scene_render_front(struct scene_t *scene)
{
    int i;
    int w, h;
    SDL_Rect d;
    main_scene_data_t *data = scene->opaque;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);

    d.x = d.y = 0;
    d.w = w;
    d.h = h / ROM_ENTRIES;
    for (i = 0; i < data->entry_cnt; i++)
    {
        draw_centered_text(scene->engine->renderer, scene->engine->font,
            black, data->entries[i].name, &d);

        d.y += d.h;
    }

}

static void
_main_scene_render_back(struct scene_t *scene)
{
    int w, h;
    SDL_Rect center;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);
    SDL_SetRenderDrawColor(scene->engine->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(scene->engine->renderer);

    /* draw center line highlight */
    center.x = center.y = 0;
    center.w = w;
    center.h = h/ROM_ENTRIES;
    center.y = center.h * (ROM_ENTRIES/2);
    SDL_SetRenderDrawColor(scene->engine->renderer, 0x0, 0x0, 0x0, 0x20);
    SDL_SetRenderDrawBlendMode(scene->engine->renderer, SDL_BLENDMODE_BLEND);

    SDL_RenderFillRect(scene->engine->renderer, &center);
}

static void
_main_scene_render_overlay(struct scene_t *scene)
{
}

static int
_main_scene_tick(struct scene_t *scene)
{
    return 1;
}

static void
_main_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
    main_scene_data_t *data = scene->opaque;

    if (event->type == SDL_CONTROLLERAXISMOTION)
    {
        if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
        {
            if (event->caxis.value > 0)
                data->offset++;
            else
                data->offset--;

            if (data->offset < 0)
                data->offset = 0;

            _main_scene_update_rom_entries(scene);

        }
    }
    else if (event->type == SDL_CONTROLLERBUTTONDOWN)
    {
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_A)
        {
            /* Do menu item action */
            if (engine_push_scene(scene->engine, &run_game_scene, &data->entries[0]) != 0)
            {
                fprintf(stderr, "Failed to run game...\n");
            }
        }
    }
}

main_scene_data_t _main_scene_data = {
    0
};

scene_t main_scene = {
    _main_scene_mount,
    _main_scene_unmount,
    _main_scene_render_back,
    _main_scene_render_front,
    _main_scene_render_overlay,
    _main_scene_tick,
    _main_scene_handle_event,
    NULL,
    &_main_scene_data
};