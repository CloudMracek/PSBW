#pragma once

#include "vector.h"
#include "Component.h"  // Include Component.h as it's used here


class GameObject {
public:
    Vector3D position;
    Vector3D rotation;
    GameObject(int x, int y, int z);
    Component* components[10];
    void execute(uint32_t* ptr);
};
