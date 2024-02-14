#include <psbw/sio.h>
#include <psbw/manager.h>

#include <psbw/Scene.h>
#include <psbw/GameObject.h>
#include <psbw/Sprite.h>
#include <psbw/Texture.h>
#include <psbw/Controller.h>
#include <psbw/Sound.h>
#include "psbw/fudgebundle.h"
#include "psbw/font.h"

extern "C" {
#include <psbw/filesystem.h>
#include <stdint.h>
#include "stdlib.h"
}

#include <vendor/printf.h>
#include <stdio.h>

#include "game/game.h"


Scene* scene1;

GameObject* gameObject1;
GameObject* gameObject2;

Sprite* sprite1;
Sprite* sprite2;

Texture* texture1;
Texture* texture2;
Texture* font;

Controller* controller1;
Sound* sound1;

FDG_HASH_ENTRY* entry;

CdlFILE file;

extern const uint8_t textureData[];
extern const uint8_t soundData[];

// Define DVD screensaver speed
int SPEED_X = 2;
int SPEED_Y = 1;

void game_setup() {
    scene1 = new Scene();
    gameObject1 = new GameObject(0, 0, 0);
    gameObject2 = new GameObject(0, 0, 0);

    sprite1 = new Sprite(SPRITE_TYPE_TEXTURED);
    sprite2 = new Sprite(SPRITE_TYPE_TEXTURED);

    controller1 = new Controller(CONTROLLER_PORT_1);


    psbw_load_scene(scene1);

    CdSearchFile(&file, "\\BUNDLE.FDG");

    //psbw_upload_texture(texture1, textureData, SCREEN_WIDTH * 2, 0, 64, 64);

    size_t len   = (file.size + 2047) & 0xfffff800;
	void   *_ptr = malloc(len);

    CdControl(CdlSetloc, &(file.pos), 0);
	CdRead(len / 2048, (uint32_t*)_ptr, CdlModeSpeed);
	if (CdReadSync(0, 0) < 0){}

    Fudgebundle *fdg = new Fudgebundle((uint8_t*)_ptr);
    texture1 = fdg->fudgebundle_get_texture(fdg_hash("sprite_cat3"));
    texture2 = fdg->fudgebundle_get_texture(fdg_hash("sprite_cat7"));
    sound1 = fdg->fudgebundle_get_sound(fdg_hash("meow"));
    //sound1 = new Sound(soundData);
    font = fdg->fudgebundle_get_texture(fdg_hash("font"));

    setFont(font);

    sprite1->tex = texture1;
    sprite1->Width = texture1->width;
    sprite1->Height = texture1->height;

    sprite2->tex = texture2;
    sprite2->Width = texture2->width;
    sprite2->Height = texture2->height;

    gameObject1->addComponent(sprite1);
    scene1->addGameObject(gameObject1);

    gameObject2->addComponent(sprite2);
    scene1->addGameObject(gameObject2);

    sound_play_cdda(2,1);
}

void game_loop() {

    // Move the game object
    gameObject1->position.x += SPEED_X;
    gameObject1->position.y += SPEED_Y;

    // Bounce off the walls
    if (gameObject1->position.x <= 0 || gameObject1->position.x + sprite1->Width >= SCREEN_WIDTH) {
        SPEED_X = -SPEED_X;
        sound1->play();
    }

    if (gameObject1->position.y <= 0 || gameObject1->position.y + sprite1->Height >= SCREEN_HEIGHT) {
        SPEED_Y = -SPEED_Y;
        sound1->play();
    }

}
