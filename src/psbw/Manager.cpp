#include "psbw/Manager.h"

#include "draw.h"

#include "psbw/Texture.h"

void psbw_load_scene(Scene* scene) {
    load_scene(scene);
}

Scene* psbw_get_active_scene() {
    return get_active_scene();
}