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