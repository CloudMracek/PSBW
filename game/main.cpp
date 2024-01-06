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

// Define DVD screensaver speed
int SPEED_X = 2;
int SPEED_Y = 1;

void game_setup() {
    scene1 = new Scene();
    gameObject1 = new GameObject(100, 100, 100);
    sprite1 = new Sprite(SPRITE_TYPE_TEXTURED);
    texture1 = new Texture();
    

    psbw_upload_texture(texture1, textureData, SCREEN_WIDTH * 2, 0, 64, 64);

    sprite1->Width = texture1->width;
    sprite1->Height = texture1->height;

    sprite1->tex = texture1;

    gameObject1->components[0] = sprite1;
    scene1->objects[0] = gameObject1;

    gameObject1->position.x = 0;
    gameObject1->position.y = 0;

    psbw_load_scene(scene1);
}

void game_loop() {
    // Move the game object
    gameObject1->position.x += SPEED_X;
    gameObject1->position.y += SPEED_Y;

    // Bounce off the walls
    if (gameObject1->position.x <= 0 || gameObject1->position.x + sprite1->Width >= SCREEN_WIDTH) {
        SPEED_X = -SPEED_X;
    }

    if (gameObject1->position.y <= 0 || gameObject1->position.y + sprite1->Height >= SCREEN_HEIGHT) {
        SPEED_Y = -SPEED_Y;
    }

}
