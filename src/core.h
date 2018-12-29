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