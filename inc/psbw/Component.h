#pragma once

#include "psbw/Vector.h"

class GameObject;

/**
 * \class Component
 * \brief An Interface class which serves to add Components to GameObject
 *
 * Do not use in your code. This in only an Interface class which parents component classes like Sprite
 */
class Component {
public:
    // Run function of selected component. Managed by engine. DO NOT RUN IN GAME CODE!
    virtual void execute(GameObject* parent) = 0;
    Vector3D relPos = {0,0,0};
};
