#include "Sprite.h"
#include "vector.h"
#include "GameObject.h"

#include <ps1/gpucmd.h>
#include <ps1/registers.h>
#include "psbw/sio.h"

void Sprite::execute(GameObject* parent, uint32_t* ptr) {
    sio_print("x");
    ptr[0] = gp0_rgb(Color.x, Color.y, Color.z) | gp0_rectangle(false, false, false);
	ptr[1] = gp0_xy(parent->position.x, parent->position.y);
	ptr[2] = gp0_xy(Width, Height);
}

Sprite::Sprite(SpriteType type) {
    Color.x = 255;
    Color.y = 255;
    Color.z = 255;

    Width = 50;
    Height = 50;

    Type = type;
}