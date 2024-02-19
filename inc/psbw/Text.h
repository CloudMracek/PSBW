#include "psbw/Component.h"
#include "psbw/Vector.h"
#include "psbw/Texture.h"
#include "psbw/GameObject.h"

class Text : public Component {
    public:
        char* text;
        void execute(GameObject* parent) override;
    
    private:
};