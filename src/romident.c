#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>

#include "romident.h"

static bool
_ident_bytes_is_ascii_visible(uint8_t *data, size_t len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        if ((*data < 0x20 || *data >= 0x7f))
            return false;
        data++;
    }

    return true;
}

static void
_ident_string_trim(uint8_t *data, size_t len)
{
    for (int i = len; i > 0; i--)
    {
        if (data[i] == ' ')
            data[i] = '\0';
    }
}

/*
 * Identify SNES ROM
 */
static int
_ident_snes_rom(int fd, romident_rom_data_t *result)
{
    struct snes_header {
        uint8_t game_title[21];
        uint8_t rom_makeup_type;
        uint8_t rom_type;
        uint8_t rom_size;
        uint8_t sram_size;
        uint8_t creator_license_id_code;
        uint16_t version;
        uint8_t compplementary_checksum[2];
        uint8_t checksum[2];
    } hdr;

    uint32_t header_offsets[] = {
        0x7fff  - 0x40 + 1,
        0xffff  - 0x40 + 1,
        0x101ff - 0x40 + 1,
        0x200 + 0x7fff - 0x40 + 1,
        0x200 + 0xffff - 0x40 + 1,
        0x200 + 0x101ff - 0x40 + 1
    };

    for (int i = 0; i < (sizeof(header_offsets) / sizeof(uint32_t)); i++)
    {
        if (lseek(fd, header_offsets[i], SEEK_SET) == -1)
            continue;

        if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
            continue;

        /* verify that header is visible ascii string*/
        if (_ident_bytes_is_ascii_visible(hdr.game_title, sizeof(hdr.game_title)) == false)
            continue;

        /* verify rom size */
        if (hdr.rom_size < 0x09 || hdr.rom_size > 0x0d)
            continue;

        goto identified;
    }

    return 1;

identified:
    memset(result, 0, sizeof(romident_rom_data_t));
    memcpy(result->name, hdr.game_title, 21);
    result->system = ROMIDENT_SNES;
    result->crc32 = *(uint32_t*)hdr.checksum;
    return 0;
}

/*
 *  Identify Sega Mega Drive ROM
 */
static int
_ident_sega_mega_drive_rom(int fd, romident_rom_data_t *result)
{
    int i;
    struct genisis_header {
        uint8_t magic[16];
        uint8_t copyright[16];
        uint8_t domestic_name[32];
        uint8_t overseas_name[32];
    } hdr;
    uint8_t buf[16*1024];

    /* detect if ROM is SMD interleaved */
    if (lseek(fd, 0x0, SEEK_SET) == -1)
        return 1;
    if (read(fd, buf, 16) != 16)
        return 1;
    if (buf[1] == 0x03 && buf[8] == 0xAA && buf[9] == 0xBB)
    {
        /* decode first interleaved 16KB block */
        if (lseek(fd, 0x200 + 0x80, SEEK_SET) == -1)
            return 1;
        if (read(fd, buf, sizeof(buf)) != sizeof(buf))
            return 1;

        uint8_t *phdr, *podd, *peven;
        phdr = (uint8_t*)&hdr;
        peven = buf;
        podd = buf + 8192;

        for (i = 0; i < sizeof(hdr) / 2; i++)
        {
            *phdr = *podd;
            phdr++;
            *phdr = *peven;
            phdr++;
            podd++;
            peven++;
        }
    }
    else
    {
        /* normal ROM dump */
        if (lseek(fd, 0x100, SEEK_SET) == -1)
            return 1;

        if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
            return 1;
    }

    /* check ROM header */
    if (strncmp((char*)hdr.magic, "SEGA MEGA DRIVE ", 16) != 0)
    {
        if (strncmp((char*)hdr.magic, "SEGA GENESIS    ", 16) != 0)
            return 1;
    }

    /* build result */
    memset(result, 0, sizeof(romident_rom_data_t));
    _ident_string_trim(hdr.domestic_name, sizeof(hdr.domestic_name));
    strcat((char*)result->name, (char*)hdr.domestic_name);
    result->system = ROMIDENT_SEGA_MEGA_DRIVE;

    return 0;
}

/*
 * Identify GAMEBOY ROM
 */
static int
_ident_game_boy_rom(int fd, romident_rom_data_t *result)
{
    uint8_t magic[] = {0x38,0x0a,0xac,0x72,0x21,0xd4,0xf8,0x07};

    struct gameboy_header {
        uint8_t magic[8];
        uint8_t title[12];
        uint8_t designation[4];
        uint8_t reserved[11];
        uint16_t checksum;
    } hdr;

    if (lseek(fd, 0xa0 - 8, SEEK_SET) == -1)
        return 1;
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
        return 1;

    /* Verify bitmap nintento logo */
    if (memcmp(hdr.magic, magic, sizeof(magic)) != 0)
        return 1;

    /* build result */
    memset(result, 0, sizeof(romident_rom_data_t));
    _ident_string_trim(hdr.title, sizeof(hdr.title));
    strcat((char*)result->name, (char*)hdr.title);
    result->crc32 = hdr.checksum;
    result->system = ROMIDENT_GAME_BOY;
    return 0;
}

static int
_romident_identify(romident_t *ident, int fd, romident_rom_data_t *result)
{
    int res;

    res = _ident_snes_rom(fd, result);
    if (res == 0)
        return 0;

    res = _ident_sega_mega_drive_rom(fd, result);
    if (res == 0)
        return 0;

    res = _ident_game_boy_rom(fd, result);
    if (res == 0)
        return 0;

    return 1;
}

int
romident_init(romident_t *ident)
{
    return 0;
}

int
romident_identify(romident_t *ident, const char *filename, romident_rom_data_t *result)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
        return 1;

    return _romident_identify(ident, fd, result);
}
