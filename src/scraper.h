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

#ifndef _scraper_h
#define _scraper_h

#include <sqlite3.h>

#include "romident.h"
#include "core.h"

typedef struct scraper_rom_entry_t  {
  const char *path;
  const char *name;
  const char *core;
} scraper_rom_entry_t;

typedef struct scraper_t {
  romident_t ident;
  sqlite3 *db;
} scraper_t;

int scraper_init(scraper_t *scraper);
void scraper_deinit(scraper_t *scraper);

int scraper_scan_directory(scraper_t *scraper, core_collection_t *core, const char *directory);

int scraper_get_list(scraper_t *scraper, uint32_t index, uint32_t limit,
                    scraper_rom_entry_t *result, size_t *size);

int32_t scraper_get_offset_to_char(scraper_t *scraper, char ch);

#endif /* _scraper_h */
