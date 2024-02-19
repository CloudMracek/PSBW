#pragma once
#include "psbw/GameObject.h"
#include "psbw/Fudgebundle.h"

typedef struct GAMEOBJECT_ENTRY {
    GameObject* object;
    GAMEOBJECT_ENTRY* next;
} GAMEOBJECT_ENTRY;


/**
 * \class Scene
 * \brief A container for all the GameObject within a Scene. The engine uses this to load data
 */

class Scene {
    public:
        char* name;
        
        Vector2D *backgroundImage = nullptr;
        
        void addGameObject(GameObject *object);
        GAMEOBJECT_ENTRY _linked_list;

        Texture* getTexture(char *name);
        Sound* getSound(char *name);
        void setBackground(char* name);

        virtual void sceneSetup() = 0;
        virtual void sceneLoop() = 0;

    protected:
        Scene(char *sceneName);
        ~Scene();
        Fudgebundle* _fdg;
};