#include "psbw/Component.h"
#include "psbw/vector.h"
#include "psbw/Texture.h"
#include "psbw/GameObject.h"

class Text : public Component {
    public:
        char* text;
        void execute(GameObject* parent) override;
    
    private:
};