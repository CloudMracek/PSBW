#include "Test3D.h"
#include "psbw/Camera.h"
#include "card.h"
#include "sio0.h"
#include <stdio.h>
#include "vsync.h"
#include <string.h>
#include <ps1/system.h>

// This code is the demonstration of the 3D capabilities. They're limited. I don't recommend.
Test3D::Test3D(char *name) : Scene(name) {type = SCENE_3D;}

Test3D::~Test3D()
{
    Scene::~Scene();
}
void Test3D::sceneSetup()
{
    Scene::camera = new Camera();
    cube = new GameObject(0,0,256);
    cubeMesh = new Mesh();
    cubeMesh->mesh = Scene::getMesh("cube");
    cubeMesh->texture = Scene::getTexture("dumbass");
    cube->addComponent(cubeMesh);
    Scene::addGameObject(cube);
    ctrl = new Controller(CONTROLLER_PORT_1);
}

void Test3D::sceneLoop()
{
    if(ctrl->GetButton(Down)) {
        Scene::camera->position.z--; 
    }
    if(ctrl->GetButton(Up)) {
        Scene::camera->position.z++; 
    }
    if(ctrl->GetButton(Left)) {
        Scene::camera->position.x--; 
    }
    if(ctrl->GetButton(Right)) {
        Scene::camera->position.x++; 
    }
}