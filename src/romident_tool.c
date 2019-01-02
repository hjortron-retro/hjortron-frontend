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

#include <stdio.h>
#include <stdlib.h>
#include "romident.h"

int main(int argc, char **argv)
{
    romident_t ident;
    romident_rom_data_t result;

    romident_init(&ident);

    /* Identify */
    if (romident_identify(&ident, argv[1], &result) != 0)
    {
        fputs("Failed to identify rom\n", stderr);
        exit(1);
    }

    /* Report */
    fprintf(stderr, "Name: %s\n"\
                    "System: %d\n"\
                    "Checksum: 0x%x\n"
                    , result.name, result.system, result.crc32);

    exit(0);
}