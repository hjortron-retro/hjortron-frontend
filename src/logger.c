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

#include <stdarg.h>
#include <stdio.h>

#include "logger.h"

extern int g_log_level;

static const char *level_to_str[] = {
    "error",
    "warning",
    "notice",
    "debug"
};

void
logger(const char *component, logger_level_t level, const char *fmt, ...)
{
    char buf[2048] = {0};
    va_list ap;
    va_start(ap, fmt);

    if  (level <= g_log_level)
        snprintf(buf, sizeof(buf), "%s(%s), %s\n", component, level_to_str[level], fmt);

    vfprintf(stderr, buf, ap);
    va_end(ap);
}