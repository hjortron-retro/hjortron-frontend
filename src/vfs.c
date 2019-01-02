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
#include <fcntl.h>
#include <unistd.h>
#include "logger.h"
#include "vfs.h"
#include "libretro.h"

struct retro_vfs_file_handle {
    int idx;
    char file[2048];
    int fd;
};

static struct retro_vfs_file_handle descriptors[32] = {0};

const char *
_vfs_unix_get_path(struct retro_vfs_file_handle *stream)
{
    return stream->file;
}

struct retro_vfs_file_handle *
_vfs_unix_open(const char *path, unsigned mode, unsigned hints)
{
    int i;
    struct retro_vfs_file_handle *stream;

    stream = descriptors;

    for (i = 0; i < 32; i++)
    {
        if (descriptors[i].fd == 0)
            break;

        stream++;
    }

    if (i == 32)
    {
        return NULL;
    }

    stream->idx = i;
    stream->fd = open(path, O_RDONLY);
    if (stream->fd == -1) {
        stream->fd = 0;
        return NULL;
    }

    snprintf(stream->file, sizeof(stream->file), "%s", path);
    debug("open file %s", path);

    return stream;
}

int
_vfs_unix_close(struct retro_vfs_file_handle *stream)
{
    close(stream->fd);
    stream->fd = 0;
    return 0;
}


int64_t
_vfs_unix_size(struct retro_vfs_file_handle *stream)
{
    off_t current = lseek(stream->fd, 0, SEEK_CUR);
    off_t size = lseek(stream->fd, 0, SEEK_END);
    lseek(stream->fd, current, SEEK_SET);
    return size;
}

int64_t
_vfs_unix_tell(struct retro_vfs_file_handle *stream)
{
    return lseek(stream->fd, 0, SEEK_CUR);
}

int64_t
_vfs_unix_seek(struct retro_vfs_file_handle *stream, int64_t offset, int seek_position)
{
    return lseek(stream->fd, offset, seek_position);
}

int64_t
_vfs_unix_read(struct retro_vfs_file_handle *stream, void *s, uint64_t len)
{
    return read(stream->fd, s, len);
}

int64_t
_vfs_unix_write(struct retro_vfs_file_handle *stream, const void *s, uint64_t len)
{
    return write(stream->fd, s, len);
}

int
_vfs_unix_flush(struct retro_vfs_file_handle *stream)
{
    return 0;
}


int
_vfs_unix_remove(const char *path)
{
    return unlink(path);
}

int
_vfs_unix_rename(const char *old_path, const char *new_path)
{
    return rename(old_path, new_path);
}

int64_t
_vfs_unix_truncate(struct retro_vfs_file_handle *stream, int64_t length)
{
    return ftruncate(stream->fd, length);
}


static struct retro_vfs_interface _vfs = {
    _vfs_unix_get_path,
    _vfs_unix_open,
    _vfs_unix_close,
    _vfs_unix_size,
    _vfs_unix_tell,
    _vfs_unix_seek,
    _vfs_unix_read,
    _vfs_unix_write,
    _vfs_unix_flush,
    _vfs_unix_remove,
    _vfs_unix_rename,
    _vfs_unix_truncate,
};

struct retro_vfs_interface *vfs_interface()
{
    return &_vfs;
}