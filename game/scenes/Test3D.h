#pragma once

#include <psbw/Scene.h>
#include <psbw/GameObject.h>
#include <psbw/Mesh.h>

class Test3D : public Scene {
    public:
        Test3D(char* name);
        ~Test3D();
        
        void sceneSetup() override;
        void sceneLoop() override;
    private:
        GameObject* cube;
        Mesh* cubeMesh;

};