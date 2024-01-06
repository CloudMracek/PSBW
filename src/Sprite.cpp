#include "Sprite.h"
#include "vector.h"
#include "GameObject.h"

#include <ps1/gpucmd.h>
#include <ps1/registers.h>
#include "psbw/sio.h"

void Sprite::execute(GameObject* parent) {
    sio_print("x");
    GPU_GP0 = gp0_rgb(Color.x, Color.y, Color.z) | gp0_rectangle(false, false, false);
	GPU_GP0 = gp0_xy(parent->position.x, parent->position.y);
	GPU_GP0 = gp0_xy(Width, Height);
}

Sprite::Sprite(SpriteType type) {
    Color.x = 255;
    Color.y = 255;
    Color.z = 255;

    Width = 50;
    Height = 50;

    Type = type;
}