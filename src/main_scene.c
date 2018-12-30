#include "scene.h"
#include "engine.h"
#include "draw.h"
#include <SDL_ttf.h>

#define ROM_ENTRIES 7

extern scene_t run_game_scene;

typedef struct {
    scraper_rom_entry_t entries[ROM_ENTRIES];
    size_t entry_cnt;
    int32_t offset;
    int32_t index;
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
    data->index = data->offset = 0;
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
    SDL_Color black = {0, 0, 0};

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);

    d.x = d.y = 0;
    d.x = 32 + 4;
    d.w = w;
    d.h = h / ROM_ENTRIES;
    for (i = 0; i < data->entry_cnt; i++)
    {
        draw_text(scene->engine->renderer, scene->engine->font,
            black, ALIGN_LEFT, data->entries[i].name, &d);

        d.y += d.h;
    }

}

static void
_main_scene_render_back(struct scene_t *scene)
{
    int w, h;
    SDL_Rect center;
    main_scene_data_t *data = scene->opaque;

    SDL_GetRendererOutputSize(scene->engine->renderer, &w, &h);
    SDL_SetRenderDrawColor(scene->engine->renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(scene->engine->renderer);

    /* draw side menu bar */
    center.x = center.y = 0;
    center.w = 32;
    center.h = h;
    SDL_SetRenderDrawColor(scene->engine->renderer, 0x0, 0x0, 0x0, 0x20);
    SDL_SetRenderDrawBlendMode(scene->engine->renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(scene->engine->renderer, &center);


    /* draw item cursor line highlight */
    center.x = 32;
    center.w = w;
    center.h = h/ROM_ENTRIES;
    center.y = data->index * center.h;
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

    if (event->type == SDL_CONTROLLERBUTTONDOWN)
    {
        switch (event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                data->index--;
                if (data->index < 0)
                {
                    data->offset--;
                    data->index = 0;
                    _main_scene_update_rom_entries(scene);
                }
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                data->index++;
                if (data->index >= ROM_ENTRIES)
                {
                    data->offset++;
                    data->index = ROM_ENTRIES - 1;
                    _main_scene_update_rom_entries(scene);
                }
                break;

            case SDL_CONTROLLER_BUTTON_A:
                if (engine_push_scene(scene->engine, &run_game_scene, &data->entries[data->index]) != 0)
                {
                    fprintf(stderr, "Failed to run game...\n");
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
    _main_scene_render_back,
    _main_scene_render_front,
    _main_scene_render_overlay,
    _main_scene_tick,
    _main_scene_handle_event,
    NULL,
    &_main_scene_data
};