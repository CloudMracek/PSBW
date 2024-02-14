#pragma once
#include <stdint.h>

#include "psbw/Texture.h"
#include "psbw/Sound.h"

typedef struct [[gnu::packed]] FDG_INDEX
{
	char magic[7]; // Magic string, must be fudgebn
    uint8_t version; // Version number, currently 2

    uint32_t indexLength; // Length of index section including padding and this header
    uint32_t vramLength; // Length of VRAM data section including padding
    uint32_t spuLength; // Length of SPU RAM data section including padding
    uint32_t ramLength; // Length of main RAM data section including padding

    uint8_t numAtlases256; // Number of 256x256 texture atlases in VRAM data section
    uint8_t numAtlases192; // Number of 192x256 texture atlases in VRAM data section
    uint8_t numAtlases128; // Number of 128x256 texture atlases in VRAM data section
    uint8_t numAtlases64; // Number of 64x256 texture atlases in VRAM data section

    uint16_t numBuckets; // Number of buckets in the hash table
    uint16_t numChained; // Number of chained entries in the hash table

} FDG_INDEX;


typedef struct [[gnu::packed]] FDG_HASH_ENTRY
{
    uint32_t hash; // Full hash of entry's name
    uint32_t offset; // Offset of entry's data within main RAM section
    uint32_t length; // Length of entry's data
    uint16_t type; // Entry type identifier (see below)
    uint16_t chained; // Index of the next chained entry in the hash table
} FDG_HASH_ENTRY;


class Fudgebundle {
    public:
        Fudgebundle(uint8_t* data);
        Texture *fudgebundle_get_texture(uint32_t hash);
        Sound *fudgebundle_get_sound(uint32_t hash);
    private:
        FDG_INDEX* _fdg_index;
        FDG_HASH_ENTRY* _hash_table;
        uint8_t* _ram_data;

        uint8_t _entry_texpage;

        int _fudgebundle_load(uint8_t* data);
        FDG_HASH_ENTRY *_fudgebundle_get_entry(uint32_t hash);
};

uint32_t fdg_hash(const char *str);
