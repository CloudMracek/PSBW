#pragma once
#include "psbw/Scene.h"
#include "psbw/Texture.h"
/**
* \brief Loads the specified scene with it's assets into ram and starts playing it. Also unloads the currently loaded scene.
*/
void psbw_load_scene(Scene* scene);
