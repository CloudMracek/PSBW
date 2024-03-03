#pragma once
#include "psbw/Scene.h"
#include "psbw/Texture.h"

#define GRAPHICS_MODE_PAL 0
#define GRAPHICS_MODE_NTSC 1

class Scene;

void load_scene(Scene* scene);
void draw_init();
void draw_update(bool doGameTick);

uint8_t draw_get_graphics_mode();

uint32_t *dma_get_chain_pointer(int numCommands, int zIndex);

void vram_send_data(const void *data, int x, int y, int width, int height);

int getOtSize();