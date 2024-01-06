#include "psbw/Component.h"
#include "psbw/vector.h"
#include "psbw/Texture.h"

#define SPRITE_TYPE_FLAT_COLOR 0
#define SPRITE_TYPE_TEXTURED 1

typedef char SpriteType;

class Sprite : public Component {
    public:

        Sprite(SpriteType type);
        Sprite(SpriteType type, Texture* texture);

        int Width, Height;
        Vector3D Color;
        SpriteType Type;
        Texture* tex;

        void execute(GameObject* parent) override;
    
    private:
};