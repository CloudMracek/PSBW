#pragma once
#include <cstdint>

class GameObject;

class Component {
public:
    virtual void execute(GameObject* parent, uint32_t* ptr) = 0;

};
