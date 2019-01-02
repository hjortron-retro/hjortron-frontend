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

#ifndef _romident_h
#define _romident_h

#include <stdint.h>

typedef enum romident_system_t
{
    ROMIDENT_UNKNOWN,
    ROMIDENT_NES,
    ROMIDENT_SNES,
    ROMIDENT_SEGA_MEGA_DRIVE,
    ROMIDENT_GAME_BOY,
} romident_system_t;

typedef struct romident_t
{
} romident_t;

typedef struct romident_rom_data_t
{
    int8_t name[64];
    romident_system_t system;
    uint32_t crc32;
} romident_rom_data_t;

int romident_init(romident_t *ident);
int romident_identify(romident_t *ident, const char *filename, romident_rom_data_t *result);

#endif /* _romident_h */