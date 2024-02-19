#include "psbw/Scene.h"

#include <stdlib.h>

Scene::Scene(char *sceneName) {
    name = sceneName;
    _linked_list.object = nullptr;
    _fdg = new Fudgebundle(sceneName);
}

Scene::~Scene() {
    delete _fdg;

    GAMEOBJECT_ENTRY *entry = &_linked_list;

    while(entry->next != nullptr) {
        entry = entry->next;
        free(entry);
    }
    free(entry);
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

Texture* Scene::getTexture(char *name) {
    return _fdg->fudgebundle_get_texture(fdg_hash(name));
}

Sound* Scene::getSound(char *name) {
    return _fdg->fudgebundle_get_sound(fdg_hash(name));
}

void Scene::setBackground(char *name) {
    backgroundImage = _fdg->fudgebundle_get_background(fdg_hash(name));
}