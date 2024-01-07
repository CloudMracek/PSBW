#include "psbw/GameObject.h"
#include <stdint.h>

GameObject::GameObject(int x, int y, int z) {
    GameObject::position.x = x;
    GameObject::position.y = y;
    GameObject::position.z = z;
}

void GameObject::execute() {
    for (int i = 0; i < 10; ++i) {
        if(components[i] != nullptr) {
            components[i]->execute(this);
        }
        else {
            break;
        }   
    }
}
