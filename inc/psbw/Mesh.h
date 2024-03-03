#pragma once

#include "psbw/Component.h"

class Mesh : public Component {
    public:
        Mesh();
        void execute(GameObject* parent) override;
    private:
};