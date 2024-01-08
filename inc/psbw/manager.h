#pragma once
#include "psbw/Scene.h"
#include "psbw/settings.h"
#include "psbw/Texture.h"

// Loads the specified scene with it's assets into ram and starts playing it. Also unloads the currently loaded scene.
void psbw_load_scene(Scene* scene);

// Upload texture from RAM into VRAM and writes details into Texture* 
void psbw_upload_texture(Texture* texture, const void* textureData, int x, int y, int width, int height);