#include "psbw/Component.h"
#include "psbw/vector.h"
#include "psbw/Texture.h"

#define SPRITE_TYPE_FLAT_COLOR 0
#define SPRITE_TYPE_TEXTURED 1

typedef char SpriteType;


/**
 * \class Sprite
 * \brief Add this component to your GameObject class to render a sprite
 */
class Sprite : public Component {
    public:

        /**
         * \brief Creates a sprite component with the selected type
        */
        Sprite(SpriteType type);

        /**
         * \brief Creates a sprite component with the selected type and assigns a texture
         */
        Sprite(SpriteType type, Texture* texture);
        
        int Width, Height;
        Vector3D Color;
        SpriteType Type;
        Texture* tex;

        /**
         * \brief Do not use - Handled by engine
         */
        void execute(GameObject* parent) override;
    
    private:
};