#pragma once
#include "psbw/GameObject.h"

/**
 * \class Scene
 * \brief A container for all the GameObject within a Scene. The engine uses this to load data
 */


typedef struct GAMEOBJECT_ENTRY {
    GameObject* object;
    GAMEOBJECT_ENTRY* next;
} GAMEOBJECT_ENTRY;

class Scene {
    public:
        char* name;
        void addGameObject(GameObject *object);
        GAMEOBJECT_ENTRY* _linked_list;

};