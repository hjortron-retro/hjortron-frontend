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

#ifndef _core_h
#define _core_h

#include <jansson.h>

#include "libretro.h"

typedef struct core_api_t {
    void *module;
    unsigned (*retro_api_version)(void);
    void (*retro_get_system_info)(struct retro_system_info *info);
    void (*retro_get_system_av_info)(struct retro_system_av_info *av);

    void (*retro_set_environment)(retro_environment_t);
    void (*retro_set_video_refresh)(retro_video_refresh_t);
    void (*retro_set_audio_sample)(retro_audio_sample_t);
    size_t (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
    void (*retro_set_input_poll)(retro_input_poll_t);
    int16_t (*retro_set_input_state)(retro_input_state_t);

    void (*retro_init)(void);
    void (*retro_deinit)(void);

    void (*retro_run)(void);
    bool (*retro_load_game)(const struct retro_game_info *game);
    void (*retro_unload_game)(void);
    void (*retro_reset)(void);
    size_t (*retro_serialize_size)(void);
    bool (*retro_serialize)(void *data, size_t size);
    bool (*retro_unserialize)(const void *data, size_t size);
} core_api_t;

typedef struct core_t {
    core_api_t api;
    json_t *variables;
    const char *name;
    const char *version;
    const char *valid_extensions;
} core_t;

int core_init(core_t *core, const char *library);
void core_variable_set(core_t *core, const char *key, const char *value);
const char *core_variable_get(core_t *core, const char *key);

/*
 * Core collection
 */
#define CORE_SLOTS 20

typedef struct core_collection_t {
    core_t slots[CORE_SLOTS];
    size_t count;
} core_collection_t;

int core_collection_init(core_collection_t *cores, const char *core_path);
size_t core_collection_size(core_collection_t *cores);
core_t *core_collection_get_by_name(core_collection_t *cores, const char *name);

#endif