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

#ifndef _config_h
#define _config_h

#include <jansson.h>

#define HJORTRON_CONFIG_FILE "hjortron.conf"

typedef struct config_t {
    json_t *root;
} config_t;

int config_init(config_t *config);
void config_deinit(config_t *config);
int config_set(config_t *config, const char *key, const char *value);
const char *config_get(config_t *config, const char *key, const char *default_value);
int config_save(config_t *config);

#endif /* _config_h */
