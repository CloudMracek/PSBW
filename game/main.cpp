#include <psbw/sio.h>
#include <psbw/manager.h>

#include <psbw/Scene.h>
#include <psbw/GameObject.h>
#include <psbw/Sprite.h>
#include <psbw/Texture.h>

#include "game/game.h"


Scene* scene1;
GameObject* gameObject1;
Sprite* sprite1;
Texture* texture1;

extern const uint8_t textureData[];

void game_setup() {

    scene1 = new Scene();
    gameObject1 = new GameObject(100,100,100);
    sprite1 = new Sprite(SPRITE_TYPE_TEXTURED);
    texture1 = new Texture();

    psbw_upload_texture(texture1, textureData, SCREEN_WIDTH*2, 0, 64, 64);

    sprite1->tex = texture1;

    gameObject1->components[0] = sprite1;
    scene1->objects[0] = gameObject1;

    gameObject1->position.x = 0;
    gameObject1->position.y = 0;

    sprite1->Width = 40;
    sprite1->Height = 40;

    psbw_load_scene(scene1);
}

void game_loop() {
    sio_print("test");
}
