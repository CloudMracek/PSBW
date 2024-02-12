#pragma once
#include "psbw/Scene.h"
#include "psbw/Texture.h"

class Scene;

void load_scene(Scene* scene);


void draw_init();
void draw_update();

uint32_t *dma_get_chain_pointer(int numCommands);

void tex_upload(
	Texture *info, const void *data, int x, int y, int width, int height
);