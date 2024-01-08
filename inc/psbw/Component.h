#pragma once

class GameObject;


// An interface class that masks components that are added to GameObjects
class Component {
public:
    // Run function of selected component. Managed by engine. DO NOT RUN IN GAME CODE!
    virtual void execute(GameObject* parent) = 0;

};
