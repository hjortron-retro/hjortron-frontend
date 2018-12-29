#include <dirent.h>
#include <dlfcn.h>
#include <memory.h>

#include "core.h"

static void
core_api_deinit(core_api_t *api)
{
    if (api->module)
        dlclose(api->module);
}

static int
core_api_init(core_api_t *api, const char *module)
{
    if (!strstr(module, "_libretro"))
        goto failure;

    api->module = dlopen(module, RTLD_LAZY|RTLD_LOCAL);
    if (api->module == NULL)
        goto failure;

    api->retro_api_version = dlsym(api->module, "retro_api_version");
    api->retro_get_system_info = dlsym(api->module, "retro_get_system_info");
    api->retro_get_system_av_info = dlsym(api->module, "retro_get_system_av_info");

    api->retro_set_environment = dlsym(api->module, "retro_set_environment");
    api->retro_set_video_refresh = dlsym(api->module, "retro_set_video_refresh");
    api->retro_set_audio_sample = dlsym(api->module, "retro_set_audio_sample");
    api->retro_set_audio_sample_batch = dlsym(api->module, "retro_set_audio_sample_batch");
    api->retro_set_input_poll = dlsym(api->module, "retro_set_input_poll");
    api->retro_set_input_state = dlsym(api->module, "retro_set_input_state");

    api->retro_init = dlsym(api->module, "retro_init");
    api->retro_deinit = dlsym(api->module, "retro_deinit");
    api->retro_run = dlsym(api->module, "retro_run");
    api->retro_load_game = dlsym(api->module, "retro_load_game");
    api->retro_unload_game = dlsym(api->module, "retro_unload_game");

    if (api->retro_get_system_info == NULL)
        goto failure;

    return 0;

failure:
    core_api_deinit(api);
    return 1;
}

int
core_init(core_t *core, const char *library)
{
    memset(core, 0, sizeof(core_t));

    core->variables = json_object();

    if (core_api_init(&core->api, library) != 0)
        return 1;

    /* get system information */
    struct retro_system_info system_info;
    core->api.retro_get_system_info(&system_info);
    core->name = system_info.library_name;
    core->version = system_info.library_version;
    core->valid_extensions = system_info.valid_extensions;

    return 0;
}

void
core_variable_set(core_t *core, const char *key, const char *value)
{
    json_t *var;
    var = json_object_get(core->variables, key);
    if (var == NULL)
    {
        var = json_object();
        json_object_set_new(core->variables, key, var);
    }

    json_object_set_new(var, "value", json_string(value));
}

const char *
core_variable_get(core_t *core, const char *key)
{
    json_t *var, *val;
    var = json_object_get(core->variables, key);
    if (!json_is_object(var))
        return NULL;

    val = json_object_get(var, "value");
    if (!json_is_string(val))
        return NULL;

    return json_string_value(val);
}

int
core_collection_init(core_collection_t *cores, const char *core_path)
{
    int i;
    char buf[512];
    core_t *core;
    DIR *dir;
    struct dirent *entry;

    dir = opendir(core_path);
    while((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
            continue;

        /* Find free core slot */
        for (i = 0; i < CORE_SLOTS; i++)
        {
            core = &cores->slots[i];
            if (core->name == NULL)
                break;
        }

        /* All slots filled, lets end loading cores */
        if (i == CORE_SLOTS)
            break;

        snprintf(buf,sizeof(buf), "%s/%s", core_path, entry->d_name);

        if (core_init(core, buf) != 0)
        {
            core->name = NULL;
            continue;
        }

        /* Core is loaded */
        cores->count++;
        printf("libretro core '%s' version '%s' loaded into slot %d\n",
                core->name, core->version, i);
    }

    return (cores->count == 0) ? 1 : 0;
}

size_t
core_collection_size(core_collection_t *cores)
{
    return cores->count;
}

core_t *
core_collection_get_by_name(core_collection_t *cores, const char *name)
{
    int i;
    for (i = 0; i < cores->count; i++)
    {
        if (strcmp(cores->slots[i].name, name) == 0)
            return &cores->slots[i];
    }
    return NULL;
}


