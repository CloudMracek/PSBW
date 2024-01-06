#include "psbw/draw.h"
#include "psbw/Texture.h"

void psbw_load_scene(Scene* scene) {
    load_scene(scene);
}

void psbw_upload_texture(Texture* texture, const void* textureData, int x, int y, int width, int height) {
    tex_upload(texture, textureData, x, y, width, height);
}