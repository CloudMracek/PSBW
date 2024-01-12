#pragma once
#include "psbw/GameObject.h"

/**
 * \class Scene
 * \brief A container for all the GameObject within a Scene. The engine uses this to load data
 */
class Scene {
    public:
     char* name;
     GameObject* objects[100];
};