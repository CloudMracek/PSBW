#pragma once

#include "psbw/Component.h"

#include "BWM.h"

class Mesh : public Component {
    public:
        Mesh();
        BWM* mesh;
        void execute(GameObject* parent) override;
    private:
};