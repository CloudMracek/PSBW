#include "psbw/Fudgebundle.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <ps1/system.h>
#include <ps1/registers.h>
#include <ps1/gpucmd.h>

#include "draw.h"
#include "cdrom.h"
#include "cdread.h"

#include "psbw/Sound.h"

#define PAGE_WIDTH 64
#define PAGE_HEIGHT 256

typedef struct [[gnu::packed]] FDG_BG_HEADER
{
    uint16_t x,y,width,height;
} FDG_BG_HEADER;

uint8_t _current_texpage = 10;

typedef struct [[gnu::packed]] FDG_TEXTURE_DESCRIPTOR
{
	uint16_t width, height, frames, mipmaps;

} FDG_TEXTURE_DESCRIPTOR;

typedef struct [[gnu::packed]] FDG_FRAME_DESCRIPTOR
{  
    uint16_t imagePageIndex, palletePageIndex;
    uint8_t xOffset, yOffset, xMargin, yMarrgin, width, height;
    uint16_t packedPalleteOffset;
    uint32_t frameFlags;
} FDG_FRAME_DESCRIPTOR;

typedef struct [[gnu::packed]] FDG_SOUND_DESCRIPTOR
{
    uint16_t leftOffset, rightOffset, length, sampleRate;
} FDG_SOUND_DESCRIPTOR;

uint32_t fdg_hash(const char *str) {
    uint32_t value = 0;

    while (*str)
        value = ((uint32_t) *(str++)) + (value << 6) + (value << 16) - value;

    return value;
}

Fudgebundle::Fudgebundle(char *filename) {
    CdlFILE file;
    CdSearchFile(&file, filename);
    size_t len = (file.size + 2047) & 0xfffff800;
    void *_ptr = malloc(len);
    CdControl(CdlSetloc, &(file.pos), 0);
    CdRead(len / 2048, (uint32_t *)_ptr, CdlModeSpeed);
    if (CdReadSync(0, 0) < 0)
    {
    }
    _fudgebundle_load((uint8_t*)_ptr);
}

Fudgebundle::~Fudgebundle() {
    free(_ram_data);
    free(_fdg_index);
    _current_texpage -= _pageCount;
}

int Fudgebundle::_fudgebundle_load(uint8_t* data) {
    _fdg_index = (FDG_INDEX*) data;

    // Copy over RAM section
    uint8_t* temp_ram_data = data+_fdg_index->indexLength+_fdg_index->vramLength+_fdg_index->spuLength;
    _ram_data = (uint8_t*) malloc(_fdg_index->ramLength);
    memcpy(_ram_data, temp_ram_data, _fdg_index->ramLength);

    if(strncmp(_fdg_index->magic, "fudgebn", 7)) {
        printf("Couldn't read fudgebundle magic.");
        return NULL;
    }

    if(_fdg_index->version != 2) {
        printf("Only version 2 fudgebundles are supported.");
        return NULL;
    }


    // Calculate pointer to VRAM data in RAM and the number of pages;
    uint8_t* vram_data = data+_fdg_index->indexLength;
    _pageCount = (_fdg_index->numAtlases256 * 4) + (_fdg_index->numAtlases192 * 3) +
    + (_fdg_index->numAtlases128 * 2) + _fdg_index->numAtlases64;

    // This is for fudgebundle stacking which is sadly broken now
    _entry_texpage = _current_texpage;
    _current_texpage = _current_texpage + _pageCount;

    // Upload data to VRAM
    for(int i = 0; i < _pageCount; i++) {
        uint8_t* currentPage = vram_data+(i*(64*256*sizeof(short)));
        if(_entry_texpage+i > 15) {
            vram_send_data(currentPage, ((_entry_texpage+i) % 16)*64, 256, PAGE_WIDTH, PAGE_HEIGHT);
        }
        else {
            vram_send_data(currentPage, ((_entry_texpage+i)*64), 0, PAGE_WIDTH, PAGE_HEIGHT);
        }
        waitForDMATransfer(DMA_GPU, 100000);
    }

    // Upload SPU samples
    spu_upload(data+_fdg_index->indexLength+_fdg_index->vramLength, _fdg_index->spuLength);


    unsigned int indexSize = _fdg_index->indexLength;
    _fdg_index = (FDG_INDEX*) malloc(indexSize);
    memcpy(_fdg_index, data, indexSize);
    _hash_table = (FDG_HASH_ENTRY*) (((uint8_t*)_fdg_index)+32);
    free(data); 

    return 0;
}

FDG_HASH_ENTRY *Fudgebundle::_fudgebundle_get_entry(uint32_t hash) {
    // As the number of buckets is always a power of 2, "hash % numBuckets" can
    // be optimized by rewriting it as "hash & (numBuckets - 1)", which is an
    // order of magnitude faster on the PS1.
    FDG_HASH_ENTRY *entry = &_hash_table[hash & (_fdg_index->numBuckets - 1)];
    if (entry->hash == hash)
        return entry;

    while (entry->chained) {
        entry = &_hash_table[entry->chained];

        if (entry->hash == hash)
            return entry;
    }

    return nullptr; // Item not found
}

Texture *Fudgebundle::fudgebundle_get_texture(uint32_t hash) {
    FDG_HASH_ENTRY *entry = _fudgebundle_get_entry(hash);
    if(entry->type != 0x0010) {
        return NULL;
    }

    FDG_TEXTURE_DESCRIPTOR *texDesc = (FDG_TEXTURE_DESCRIPTOR*) (_ram_data+entry->offset);
    FDG_FRAME_DESCRIPTOR *frameDesc = (FDG_FRAME_DESCRIPTOR*) (_ram_data+entry->offset+sizeof(FDG_TEXTURE_DESCRIPTOR));

    Texture *tex = new Texture();

    int widthDivider;
    if((frameDesc->frameFlags & 0x3) != 2) {
       widthDivider = ((frameDesc->frameFlags & 0x3) == GP0_COLOR_8BPP) ? 2 : 4;
    }
    else {
        widthDivider = 1;
    }

    tex->width = frameDesc->width;
    tex->height = frameDesc->height;

    int globalX, globalY;

    if(frameDesc->imagePageIndex <= 6) {
        globalX = (frameDesc->imagePageIndex+10)*64 + tex->u;
        globalY = tex->v;
    }
    else {
        globalX = (frameDesc->imagePageIndex-7)*64;
        globalY = tex->v+256;
    }

    uint8_t mode = (frameDesc->frameFlags & 0x3);

    switch(mode) {
        case 0:
            widthDivider = 4;
            tex->page = gp0_page(
		        globalX / 64, globalY / 256, GP0_BLEND_SEMITRANS, GP0_COLOR_4BPP
	        );
            break;
        case 1:
            widthDivider = 2;
            tex->page = gp0_page(
		        globalX / 64, globalY / 256, GP0_BLEND_SEMITRANS, GP0_COLOR_8BPP
	        );
            break;
        case 2:
            widthDivider = 1;
            tex->page = gp0_page(
		        globalX / 64, globalY / 256, GP0_BLEND_SEMITRANS, GP0_COLOR_16BPP
	        );
            break;
    }
    
    tex->u = frameDesc->xOffset*widthDivider;
    tex->v = frameDesc->yOffset;

    if(frameDesc->frameFlags & 0x3 == 2) {
        tex->clut = 0;
    }
    else if((frameDesc->frameFlags & 0x3) == 0 || (frameDesc->frameFlags & 0x3) == 1) {
        uint16_t pageOffset = ((((_entry_texpage + frameDesc->palletePageIndex)%16)*64) / 16) | ((((_entry_texpage + frameDesc->palletePageIndex) < 16) ? 0 : 64) << 6);
        tex->clut = frameDesc->packedPalleteOffset+pageOffset;
        for(int j = 1; j < 10000; j++);
    }
    tex->type = frameDesc->frameFlags & 0x3;

    return tex;
}

Sound *Fudgebundle::fudgebundle_get_sound(uint32_t hash) {
    FDG_SOUND_DESCRIPTOR *soundDesc;
    FDG_HASH_ENTRY *entry;
    entry = _fudgebundle_get_entry(hash);
    soundDesc = (FDG_SOUND_DESCRIPTOR*) (_ram_data+entry->offset);
    Sound* snd = new Sound();
    snd->soundAddr = soundDesc->leftOffset;
    snd->sampleRate = soundDesc->sampleRate;
    return snd;
}

Vector2D *Fudgebundle::fudgebundle_get_background(uint32_t hash) {
    FDG_HASH_ENTRY *entry;
    entry = _fudgebundle_get_entry(hash);

    FDG_BG_HEADER *header = (FDG_BG_HEADER*)(_ram_data+entry->offset);

    // TODO: WASTEFUL!!!
    if(_current_texpage+5 > 15 && _current_texpage <= 15) {
        _current_texpage += 5;
        _pageCount += 5;
    }

    unsigned int globalX, globalY;
    if(_current_texpage <= 15) {
        globalX = (_current_texpage)*64;
        globalY = 0;
    }
    else {
        globalX = (_current_texpage-16)*64;
        globalY = 256;
    }

    vram_send_data(_ram_data+entry->offset+sizeof(FDG_BG_HEADER), globalX, globalY, header->width, header->height);

    _current_texpage+=5;
    _pageCount += 5;
    Vector2D *out = (Vector2D*)malloc(sizeof(Vector2D));

    out->x = globalX;
    out->y = globalY;

    return out;
}