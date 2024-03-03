#pragma once

#include "psbw/Component.h"

#include "bwm.h"

class Mesh : public Component {
    public:
        Mesh();
        BWM* mesh;
        void execute(GameObject* parent) override;
    private:
};