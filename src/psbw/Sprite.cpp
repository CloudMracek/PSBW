#include "psbw/Sprite.h"

#include <ps1/gpucmd.h>
#include <ps1/registers.h>

#include "draw.h"

#include "psbw/Vector.h"
#include "psbw/GameObject.h"

void Sprite::execute(GameObject* parent) {

    uint32_t* ptr;
    if(Type == SPRITE_TYPE_FLAT_COLOR) {
        ptr = dma_get_chain_pointer(3, zIndex);
        ptr[0] = gp0_rgb(Color.x, Color.y, Color.z) | gp0_rectangle(false, false, false);
	    ptr[1] = gp0_xy(parent->position.x+Component::relPos.x, parent->position.y+Component::relPos.y);
	    ptr[2] = gp0_xy(Width, Height);
    }
    else if(Type == SPRITE_TYPE_TEXTURED && tex != nullptr) {
        ptr = dma_get_chain_pointer(5, zIndex);
        ptr[0] = gp0_texpage(tex->page, false, false);
		ptr[1] = gp0_rectangle(true, true, false);
		ptr[2] = gp0_xy(parent->position.x, parent->position.y);
        if(tex->type == 0 || tex->type == 1) {
 		    ptr[3] = gp0_uv(tex->u, tex->v, tex->clut);
        }
        else {
            ptr[3] = gp0_uv(tex->u, tex->v, 0);
        }
		ptr[4] = gp0_xy(tex->width, tex->height);
    }
}

Sprite::Sprite(SpriteType type) {
    Color.x = 255;
    Color.y = 255;
    Color.z = 255;

    Width = 64;
    Height = 64;

    Type = type;
}

Sprite::Sprite(SpriteType type, Texture* texture) {
    Color.x = 255;
    Color.y = 255;
    Color.z = 255;

    Width = 50;
    Height = 50;

    Type = type;
    tex = texture;
}