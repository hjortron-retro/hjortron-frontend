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