#include "psbw/GameObject.h"

#include <stdint.h>
#include <stdlib.h>


GameObject::GameObject(int x, int y, int z) {
    GameObject::position.x = x;
    GameObject::position.y = y;
    GameObject::position.z = z;
    _linked_list.component = nullptr;
}

void GameObject::addComponent(Component *component) {
    if(_linked_list.component == nullptr) {
        COMPONENT_ENTRY *entry = (COMPONENT_ENTRY*) malloc(sizeof(COMPONENT_ENTRY));
        entry->component = component;
        entry->next = nullptr;
        _linked_list = *entry;
        free(entry);
        return;
    }

    COMPONENT_ENTRY *entry = &_linked_list;

    while(entry->next != nullptr) {
        entry = entry->next;
    }

    COMPONENT_ENTRY *new_entry = (COMPONENT_ENTRY*) malloc(sizeof(COMPONENT_ENTRY));
    new_entry->component = component;
    new_entry->next = NULL;

    entry->next = new_entry;

}

void GameObject::execute() {
   
   COMPONENT_ENTRY *entry = &_linked_list;
    while(entry != nullptr) {
        entry->component->execute(this);
        entry = entry->next;
    }
}
