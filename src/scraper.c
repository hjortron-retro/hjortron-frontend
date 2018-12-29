#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>

#include "scraper.h"

static bool
_scraper_db_has_rom_table(scraper_t *scraper)
{
  int res;
  res = sqlite3_exec(scraper->db,
		   "SELECT * FROM roms",
		     NULL, NULL, NULL);
  if (res == SQLITE_OK)
    return true;

  return false;
}

static int
_scraper_db_create_rom_table(scraper_t *scraper)
{
  int res;
  res = sqlite3_exec(scraper->db,
		     "CREATE TABLE roms ("			\
		     " path        TEXT UNIQUE,"		\
		     " name        TEXT,"			\
		     " description TEXT,"			\
		     " crc32       INTEGER UNIQUE,"		\
		     " core        TEXT"			\
		     ");", NULL, NULL, NULL);

  if (res != SQLITE_OK)
  {
    return 1;
  }

  return 0;
}

static int
_scraper_db_add_rom(scraper_t *scraper, core_t *core, const char *filename, const char *name)
{
  sqlite3_stmt *insert_stmt;
  char query[512]={0};

  // fprintf(stderr, "Adding rom: '%s'\n", filename);

  strcat(query, "INSERT INTO roms(path, core, name) VALUES(?, ?, ?)");

  if (sqlite3_prepare_v2(scraper->db, query, sizeof(query), &insert_stmt, NULL) != SQLITE_OK)
  {
    return 1;
  }

  sqlite3_bind_text(insert_stmt, 1, filename, -1, SQLITE_STATIC);
  sqlite3_bind_text(insert_stmt, 2, core->name, -1, SQLITE_STATIC);
  sqlite3_bind_text(insert_stmt, 3, name, -1, SQLITE_STATIC);


  if (sqlite3_step(insert_stmt) != SQLITE_OK)
  {
    sqlite3_finalize(insert_stmt);
    return 1;
  }

  sqlite3_finalize(insert_stmt);
  return 0;
}

#define safe_strdup(str) (str?strdup((char *)str):NULL)

static int
_scraper_db_get_list(scraper_t *scraper, uint32_t offset, uint32_t limit,
                    scraper_rom_entry_t *result, size_t *size)
{
    int rc;
    size_t s;
    sqlite3_stmt *get_list_stmt;
    char query[512] = {};

    strcat (query, "SELECT path,name,core FROM roms ORDER BY path LIMIT ? OFFSET ?");

    if (sqlite3_prepare_v2(scraper->db, query, sizeof(query), &get_list_stmt, NULL) != SQLITE_OK)
    {
        return 1;
    }

    sqlite3_bind_int(get_list_stmt, 1, limit);
    sqlite3_bind_int(get_list_stmt, 2, offset);

    s = *size;
    *size = 0;

    while((rc = sqlite3_step(get_list_stmt) == SQLITE_ROW) && *size < s)
    {
        result[*size].path = safe_strdup(sqlite3_column_text(get_list_stmt, 0));
        result[*size].name = safe_strdup(sqlite3_column_text(get_list_stmt, 1));
        result[*size].core = safe_strdup(sqlite3_column_text(get_list_stmt, 2));

        (*size)++;
    }

    sqlite3_finalize(get_list_stmt);
    return 0;
}


int
scraper_init(scraper_t *scraper)
{
  memset(scraper, 0, sizeof(scraper_t));
  if (sqlite3_open("./hjortron.db", &scraper->db) != SQLITE_OK)
  {
    return 1;
  }

  if (!_scraper_db_has_rom_table(scraper))
  {
    if (_scraper_db_create_rom_table(scraper) != 0)
      return 1;
  }

  return 0;
}

void
scraper_deinit(scraper_t *scraper)
{
  sqlite3_close(scraper->db);
}

static bool
_scraper_file_match_extensions(const char *filename, const char *extensions)
{
    char ext[64];
    const char *ps, *pd;

    if (extensions == NULL)
        return false;

    ps = strrchr(filename, '.');
    if (ps == NULL)
        return false;

    snprintf(ext, sizeof(ext), "%s", ps + 1);

    ps = extensions;
    while(*ps != '\0')
    {
        pd = ext;

        /* compare ext with entry */
        while(*ps != '\0' && *ps != '|')
        {
            if (*pd == '\0')
                break;

            if (*ps == '|')
                break;

            if (*ps != *pd)
                break;

            ps++;
            pd++;
        }

        /* do we have a matching extension */
        if (*pd == '\0' && *ps == '|')
            return true;

        /* advance to next extension for matching */
        while(*ps != '\0' && *ps != '|')
            ps++;

        if (*ps == '|')
            ps++;
    }

    return false;
}

int
scraper_scan_directory(scraper_t *scraper, core_collection_t *cores, const char *directory)
{
    int i;
    DIR *dir;
    core_t *core;
    char path[4096];
    struct dirent *entry;

    dir = opendir(directory);
    if (dir == NULL)
        return 1;

    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        if (entry->d_type == DT_DIR
            && strcmp(entry->d_name,".") != 0
            && strcmp(entry->d_name, "..") != 0)
        {
            scraper_scan_directory(scraper, cores, path);
            continue;
        }

        if (entry->d_type != DT_REG)
            continue;

        /* look up which core that matches rom file */
        core = NULL;
        for (i = 0; i < core_collection_size(cores); i++)
        {
            if (_scraper_file_match_extensions(entry->d_name, cores->slots[i].valid_extensions) == true)
            {
                core = &cores->slots[i];
                break;
            }
        }

        if (core == NULL)
            continue;

        /* TODO: if file is zip, then scan content */

        /* TODO: calculate crc32 */

        /* TODO: consider lookup rom in rdb by crc32 for additional info */
        char *ps;
        char name[1024];
        snprintf(name, sizeof(name), "%s", entry->d_name);
        ps = strrchr(name, '.');
        if (ps)
          *ps='\0';

        _scraper_db_add_rom(scraper, core, path, name);
    }

  return 0;
}

int
scraper_get_list(scraper_t *scraper, uint32_t index, uint32_t limit,
                    scraper_rom_entry_t *result, size_t *size)
{
    return _scraper_db_get_list(scraper, index, limit, result, size);
}
