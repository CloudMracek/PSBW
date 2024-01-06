#pragma once
#include "psbw/Scene.h"
#include "psbw/settings.h"
#include "psbw/Texture.h"
void psbw_load_scene(Scene* scene);
void psbw_upload_texture(Texture* texture, const void* textureData, int x, int y, int width, int height);