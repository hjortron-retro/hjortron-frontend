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

#include <inttypes.h>
#include <memory.h>
#include "config.h"

const char *
_config_path_get_key_name(const char *key, char *result, size_t size)
{
    const char *ps;

    if (key[strlen(key)-1] == '/')
        return NULL;

    ps = key + strlen(key) - 1;
    while (ps >= key && *ps != '/')
        ps--;

    snprintf(result, size, "%s", ps + 1);

    return result;
}

const char *
_config_path_get_component(const char *key, uint8_t index, char *result, size_t size)
{
    int i;
    const char *ps, *pe;
    if (key[0] != '/')
        return NULL;

    i = 0;
    ps = key;
    while(1)
    {
        while (*ps != '\0' && *ps != '/')
            ps++;

        ps++;

        if (*ps == '\0')
            return NULL;

        if (i < index)
        {
            i++;
            continue;
        }

        /* get component */
        pe = ps;
        while(*pe != '\0' && *pe != '/')
            pe++;

        if (*pe == '\0')
            return NULL;

        pe++;

        if (size > pe - ps)
            size = pe - ps;

        snprintf(result, size, "%s", ps);
        break;
    }

    return result;
}

json_t *
_config_get_container(config_t *config, const char *key, int create)
{
    int i;
    json_t *object;
    char buf[1024];

    i = 0;
    object = config->root;

    while (_config_path_get_component(key, i, buf, sizeof(buf)))
    {
        json_t *o;

        /* key object @ key */
        o = json_object_get(object, buf);
        if (o == NULL)
        {
            if (create)
            {
                o = json_object();
                json_object_set(object, buf, o);
            }
            else
                return NULL;
        }


        if (!json_is_object(o))
            return NULL;

        object = o;

        i++;
    }

    return object;
}

int
config_init(config_t *config)
{
    json_error_t err;
    memset(config,0, sizeof(config_t));

    config->root = json_load_file(HJORTRON_CONFIG_FILE, 0,&err);
    if (config->root == NULL)
    {
        config->root = json_object();
    }

    return 0;
}

void
config_deinit(config_t *config)
{
  config_save(config);
}

int
config_save(config_t *config)
{
    int res;
    res = json_dump_file(config->root, HJORTRON_CONFIG_FILE, JSON_INDENT(2) | JSON_COMPACT);
    if (res != 0)
        return 1;

    return 0;
}

int
config_set(config_t *config, const char *key, const char *value)
{
    char buf[512];
    json_t *object;

    object = _config_get_container(config, key, 1);
    if (object == NULL)
        return 1;

    if (!json_is_object(object))
        return 1;

    json_object_set(object, _config_path_get_key_name(key, buf, sizeof(buf)), json_string(value));

    return 0;
}

const char *
config_get(config_t *config, const char *key, const char *default_value)
{
    char buf[512];
    json_t *object, *value;

    object = _config_get_container(config, key, 0);
    if (object == NULL)
         return default_value;


    value = json_object_get(object, _config_path_get_key_name(key, buf, sizeof(buf)));
    if (value == NULL)
        return default_value;

    if (!json_is_string(value))
        return default_value;

    return json_string_value(value);
}
