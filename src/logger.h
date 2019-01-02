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

#ifndef _logger_h
#define _logger_h

typedef enum logger_level_t
{
    LOG_ERROR = 0,
    LOG_WARNING,
    LOG_NOTICE,
    LOG_DEBUG
} logger_level_t;

void logger(const char *component, logger_level_t level, const char *fmt, ...);

#define debug(fmt, ...) \
    logger(__func__, LOG_DEBUG, fmt, ##__VA_ARGS__)

#define notice(component, fmt, ...) \
    logger(component, LOG_NOTICE, fmt, ##__VA_ARGS__)

#define warning(component, fmt, ...) \
    logger(component, LOG_WARNING, fmt, ##__VA_ARGS__)

#define error(component, fmt, ...) \
    logger(component, LOG_ERROR, fmt, ##__VA_ARGS__)

#endif /* _logger_h */