#pragma once

#include "psbw/Component.h"
#include "psbw/Texture.h"

#include "BWM.h"

class Mesh : public Component {
    public:
        Mesh();
        BWM* mesh;
        Texture* texture = nullptr;
        void execute(GameObject* parent) override;
    private:
};