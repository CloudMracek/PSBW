#pragma once

class GameObject;

class Component {
public:
    virtual void execute(GameObject* parent) = 0;

};
