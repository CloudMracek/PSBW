#include "GameObject.h"
#include "psbw/sio.h"

GameObject::GameObject(int x, int y, int z) {
    GameObject::position.x = x;
    GameObject::position.y = y;
    GameObject::position.z = z;
}

void GameObject::execute() {
    sio_print("x");
    for (int i = 0; i < 10; ++i) {
        if(components[i] != nullptr) {
            components[i]->execute(this);
        }
    }
}
