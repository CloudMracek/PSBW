#include "psbw/Component.h"
#include "psbw/vector.h"

#define SPRITE_TYPE_FLAT_COLOR 0
#define SPRITE_TYPE_TEXTURED 1

typedef char SpriteType;

class Sprite : public Component {
    public:

        Sprite(SpriteType type);

        int Width, Height;
        Vector3D Color;
        SpriteType Type;

        void execute(GameObject* parent, uint32_t* ptr) override;
    
    private:
};