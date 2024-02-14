#pragma once

#include "psbw/vector.h"
#include "psbw/Component.h"  // Include Component.h as it's used here

/**
 * \class GameObject
 * \brief a GameObject which can have components assigned
 */

typedef struct COMPONENT_ENTRY {
    Component* component;
    COMPONENT_ENTRY* next;
} COMPONENT_ENTRY;

class GameObject {
public:
    Vector3D position;
    Vector3D rotation;
    GameObject(int x, int y, int z);
    /**
     * \brief Do not use. This function is critical to be called at the right time and it's handled by the engine
    */
    void execute();
    void addComponent(Component* component);
private:
    COMPONENT_ENTRY* _linked_list;
};
