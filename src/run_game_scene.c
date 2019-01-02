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

#include "logger.h"
#include "scene.h"
#include "engine.h"
#include "draw.h"
#include "vfs.h"
#include <SDL_ttf.h>
#include <asoundlib.h>

extern scene_t in_game_menu_scene;

typedef struct {
    engine_t *engine;
    SDL_Texture *screen;
    scraper_rom_entry_t *rom_entry;
    struct core_t *core;
    snd_pcm_t *pcm;
    int width;
    int height;
    uint16_t joypad_state;
} run_game_scene_data_t;

run_game_scene_data_t _run_game_scene_data;

static void
_run_game_retro_log_printf(enum retro_log_level level, const char *fmt, ...)
{
    char buf[1024] = {0};
    static char last[1024] = {0};
    static enum retro_log_level last_level;
    static uint8_t counter = 0;
    va_list args;

    /* TODO: do we want to have log level to be configurable ? */
    if (level == 0)
        return;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (strcmp(buf, last) == 0 && last_level == level)
    {
        counter++;
    }
    else
    {
        if (counter != 0)
            fprintf(stderr, "Last message repeated %d times\n", counter);
        counter = 0;
    }

    if (counter == 0)
    {
        fprintf(stderr, "RETRO[%d]: %s", level, buf);

        snprintf(last, sizeof(last), "%s", buf);
        last_level = level;
        counter = 0;
    }

}

static void
_run_game_retro_video_refresh_callback(const void *data, unsigned width, unsigned height, size_t pitch)
{
    engine_t *engine = _run_game_scene_data.engine;
    void *tdata;
    int tpitch;
    SDL_Rect rect;

    if (_run_game_scene_data.screen == NULL
        || (_run_game_scene_data.width != width || _run_game_scene_data.height != height))
    {
        if (_run_game_scene_data.screen)
            SDL_DestroyTexture(_run_game_scene_data.screen);

        _run_game_scene_data.screen = SDL_CreateTexture(engine->renderer,
            SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, width, height);

        _run_game_scene_data.width = width;
        _run_game_scene_data.height = height;
    }

    rect.x = rect.y = 0;
    rect.w = width;
    rect.h = height;

    SDL_LockTexture(_run_game_scene_data.screen, &rect, &tdata, &tpitch);

    const uint8_t *src;
    uint8_t *dst;
    src = data;
    dst = tdata;
    for (int y = 0; y < height; y++)
    {
        memcpy(dst, src, tpitch);
        dst += tpitch;
        src += pitch;
    }

    SDL_UnlockTexture(_run_game_scene_data.screen);
}

static void
_run_game_retro_audio_sample_callback(int16_t left, int16_t right)
{
    /* TODO */
}

static size_t
_run_game_retro_audio_sample_batch_callback(const int16_t *data, size_t frames)
{
    int res;
    while(1)
    {
        res = snd_pcm_writei(_run_game_scene_data.pcm, data, frames);
        if (res > 0)
            break;

        snd_pcm_recover(_run_game_scene_data.pcm, res, 0);
    }

    return frames;
}

static void
_run_game_retro_input_poll_callback(void)
{
    /* TODO */
}

static int16_t
_run_game_retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id)
{
    if (device == RETRO_DEVICE_JOYPAD && port == 0)
    {
        debug("State 0x%x, Port %d, device %d. index %d, id %d\n",
            (_run_game_scene_data.joypad_state >> id) & 1, port, device, index, id);
        return (_run_game_scene_data.joypad_state >> id) & 1;
    }
    return 0;
}


static bool
_run_game_retro_environment_callback(unsigned cmd, void *data)
{
    core_t *core = _run_game_scene_data.core;

    // cmd &= ~(RETRO_ENVIRONMENT_EXPERIMENTAL);

    switch (cmd)
    {
        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
        {
            bool *pval = data;
            *pval = true;
            return true;
        } break;

        case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
        {
            const unsigned *pval = data;
            notice("run_game_scene","  Performance level hint: %d\n", *pval);
            return true;
        } break;

        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:
        {
            const bool *pval = data;
            notice("run_game_scene", "  Core can run without game data: %s\n", *pval?"true":"false");
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES:
        {
            uint64_t *pval = data;
            *pval = (1 << RETRO_DEVICE_JOYPAD);
            return true;
        } break;

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        {
            enum retro_pixel_format *pval = data;
            notice("run_game_scene", "  Pixel format: %d\n", *pval);
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        {
            const char **pval =data;
            *pval = "./";
            return true;
        } break;

        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
        {
            const struct retro_input_descriptor *pval = data;
            notice("run_game_scene", "  Input Descriptors:\n");
            while(pval->description)
            {
                notice("run_game_scene", "      %d,%d,%d,%d: %s\n",
                            pval->port, pval->device,
                            pval->index, pval->id, pval->description);
                pval++;
            }
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        {
            struct retro_log_callback *pval = data;
            pval->log = _run_game_retro_log_printf;
            return true;
        } break;

        case RETRO_ENVIRONMENT_SET_VARIABLES:
        {
            struct retro_variable *pvar = data;
            while(pvar->key)
            {
                char value[64] = {};
                char *ps, *pe;
                pe = strchr(pvar->value, '|');
                if (pe == NULL)
                {
                    pvar++;
                    continue;
                }

                ps = pe;
                while (*ps != ' ') ps--;
                ps++;
                strncat(value, ps, pe-ps);

                core_variable_set(core, pvar->key, value);
                pvar++;
            }
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_VARIABLE:
        {
             struct retro_variable *pvar = data;
             pvar->value = core_variable_get(core, pvar->key);
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
        {
            bool *presult = data;
            *presult = false;
            return true;
        } break;

        case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:
        {
            struct retro_subsystem_info *pss = data;
            notice("run_game_scene", "  Subsystem: %s (%d roms)\n", pss->desc, pss->num_roms);
            return true;
        } break;

        case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
        {
	    //struct retro_controller_info *pctrl = data;
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        {
            const char **pval = data;
            *pval = "/tmp";
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_VFS_INTERFACE:
        {
            struct retro_vfs_interface_info *pval = data;
            pval->iface = vfs_interface();
            return true;
        } break;

        case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE:
        {
            int *pval = data;

            *pval = (1<<0) | (1<<1);
            return true;
        } break;

        default:
            warning("run_game_scene", "unhandled environment command %d", cmd);
            break;
    }
    return false;
}
#include <sys/stat.h>
static int
_run_game_scene_mount(struct scene_t *scene, void *opaque)
{
    struct retro_game_info game = {0};
    run_game_scene_data_t *data = scene->opaque;

    data->engine = scene->engine;

    data->rom_entry = opaque;
    data->core = core_collection_get_by_name(&scene->engine->cores, data->rom_entry->core);

    if (data->core == NULL)
        return 1;

    data->core->api.retro_set_environment(_run_game_retro_environment_callback);
    //json_dumpfd(data->core->variables, 0, 0);

    data->core->api.retro_set_video_refresh(_run_game_retro_video_refresh_callback);
    data->core->api.retro_set_audio_sample(_run_game_retro_audio_sample_callback);
    data->core->api.retro_set_audio_sample_batch(_run_game_retro_audio_sample_batch_callback);
    data->core->api.retro_set_input_poll(_run_game_retro_input_poll_callback);
    data->core->api.retro_set_input_state(_run_game_retro_input_state_callback);

    data->core->api.retro_init();

    data->width = data->height = 0;

    game.path = data->rom_entry->path;
    data->core->api.retro_load_game(&game);

    struct retro_system_av_info av;
    data->core->api.retro_get_system_av_info(&av);

    int err;
    if ((err = snd_pcm_open(&data->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        error("run_game_scene", "failed to open playback device %s", snd_strerror(err));
        return 1;
    }

    err = snd_pcm_set_params(data->pcm, SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED,
        2, av.timing.sample_rate, 1, 64 * 1000);
    if (err < 0)
    {
        error("run_game_scene", "Failed to configure audio device: %s", snd_strerror(err));
        return 1;
    }

    return 0;
}

static void
_run_game_scene_unmount(struct scene_t *scene)
{
    run_game_scene_data_t *data = scene->opaque;
    snd_pcm_close(data->pcm);
    data->core->api.retro_unload_game();
    data->core->api.retro_deinit();
}

static void
_run_game_scene_enter(struct scene_t *scene)
{
    run_game_scene_data_t *data = scene->opaque;
    snd_pcm_pause(data->pcm, 0);
}

static void
_run_game_scene_leave(struct scene_t *scene)
{
    run_game_scene_data_t *data = scene->opaque;
    snd_pcm_drain(data->pcm);
    snd_pcm_pause(data->pcm, 1);
}


static void
_run_game_scene_render_front(struct scene_t *scene)
{
}

static void
_run_game_scene_render_back(struct scene_t *scene)
{
    run_game_scene_data_t *data = scene->opaque;
    SDL_RenderCopy(scene->engine->renderer, data->screen, NULL, NULL);
}

static void
_run_game_scene_render_overlay(struct scene_t *scene)
{
}

static int
_run_game_scene_tick(struct scene_t *scene)
{
    run_game_scene_data_t *data = scene->opaque;
    //SDL_Delay(1);
    data->core->api.retro_run();
    return 1;
}

static void
_run_game_scene_handle_event(struct scene_t *scene, SDL_Event *event)
{
    run_game_scene_data_t *data = scene->opaque;
    if (event->type == SDL_CONTROLLERAXISMOTION)
    {
        /* SDL Event into Joystick state */
        if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
        {
            /* Clear left/right bits */
            data->joypad_state &= ~((1 << RETRO_DEVICE_ID_JOYPAD_LEFT) || (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT));

            /* Update */
            if (event->caxis.value > 0)
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT);
            else if (event->caxis.value < 0)
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_LEFT);
        }
        else if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
        {
            /* Clear uo/down bits */
            data->joypad_state &= ~((1 << RETRO_DEVICE_ID_JOYPAD_UP) || (1 << RETRO_DEVICE_ID_JOYPAD_DOWN));

            /* Update */
            if (event->caxis.value > 0)
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_UP);
            else if (event->caxis.value < 0)
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_DOWN);
        }
    }
    else if (event->type == SDL_CONTROLLERBUTTONDOWN)
    {
        /*
         * Map SDL GameController button down event into retro joystick
         * states
         */
        switch(event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_UP);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_DOWN);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_LEFT);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT);
                break;
            case SDL_CONTROLLER_BUTTON_A:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_A);
                break;
            case SDL_CONTROLLER_BUTTON_B:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_B);
                break;
            case SDL_CONTROLLER_BUTTON_X:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_X);
                break;
            case SDL_CONTROLLER_BUTTON_Y:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_Y);
                break;
            case SDL_CONTROLLER_BUTTON_GUIDE:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_SELECT);
                break;
            case SDL_CONTROLLER_BUTTON_START:
                data->joypad_state |= (1 << RETRO_DEVICE_ID_JOYPAD_START);
                break;
        }

        /* Handle special case for in game menu access */
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_BACK)
        {
            engine_push_scene(scene->engine, &in_game_menu_scene, data->core);
        }
    }
    else if (event->type == SDL_CONTROLLERBUTTONUP)
    {
        /*
         * Map SDL GameController button up event into retro joystick
         * states
         */
        switch(event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_UP);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_DOWN);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_LEFT);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_RIGHT);
                break;
            case SDL_CONTROLLER_BUTTON_A:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_A);
                break;
            case SDL_CONTROLLER_BUTTON_B:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_B);
                break;
            case SDL_CONTROLLER_BUTTON_X:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_X);
                break;
            case SDL_CONTROLLER_BUTTON_Y:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_Y);
                break;
            case SDL_CONTROLLER_BUTTON_GUIDE:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_SELECT);
                break;
            case SDL_CONTROLLER_BUTTON_START:
                data->joypad_state &= ~(1 << RETRO_DEVICE_ID_JOYPAD_START);
                break;
        }
    }
}

scene_t run_game_scene = {
    _run_game_scene_mount,
    _run_game_scene_unmount,
    _run_game_scene_enter,
    _run_game_scene_leave,
    _run_game_scene_render_back,
    _run_game_scene_render_front,
    _run_game_scene_render_overlay,
    _run_game_scene_tick,
    _run_game_scene_handle_event,
    NULL,
    &_run_game_scene_data
};
