#include "Test3D.h"

Test3D::Test3D(char *name) : Scene(name) {type = SCENE_3D;}

Test3D::~Test3D()
{
    Scene::~Scene();
}

void Test3D::sceneSetup()
{
    cube = new GameObject(0,0,0);
    cubeMesh = new Mesh();
    cubeMesh->mesh = Scene::getMesh("cube");
    cubeMesh->texture = Scene::getTexture("dumbass");
    cube->addComponent(cubeMesh);
    Scene::addGameObject(cube);
}

void Test3D::sceneLoop()
{
}