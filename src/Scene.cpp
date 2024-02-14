#include "psbw/Scene.h"

#include <stdlib.h>

Scene::Scene() {
    _linked_list.object = nullptr;
}

void Scene::addGameObject(GameObject *object) {
    if(_linked_list.object == nullptr) {
        GAMEOBJECT_ENTRY *entry = (GAMEOBJECT_ENTRY*) malloc(sizeof(GAMEOBJECT_ENTRY));
        entry->object = object;
        entry->next = nullptr;
        _linked_list = *entry;
        free(entry);
        return;
    }

    GAMEOBJECT_ENTRY *entry = &_linked_list;

    while(entry->next != nullptr) {
        entry = entry->next;
    }

    GAMEOBJECT_ENTRY *new_entry = (GAMEOBJECT_ENTRY*) malloc(sizeof(GAMEOBJECT_ENTRY));
    new_entry->object = object;
    new_entry->next = NULL;

    entry->next = new_entry;

}