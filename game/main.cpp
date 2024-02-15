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

Texture *blue, *green, *orange, *purple, *red, *turqoise, *yellow;

Controller* controller1;
Sound* sound1;

FDG_HASH_ENTRY* entry;

CdlFILE file;


#define FIELD_X 110
#define FIELD_Y 20

#define FIELD_ROWS 20
#define FIELD_COLS 10
#define CELL_SIZE 10

GameObject *array[FIELD_ROWS][FIELD_COLS];

void game_setup() {

    scene1 = new Scene();

    CdSearchFile(&file, "\\BUNDLE.FDG");
    size_t len   = (file.size + 2047) & 0xfffff800;
	void   *_ptr = malloc(len);
    CdControl(CdlSetloc, &(file.pos), 0);
	CdRead(len / 2048, (uint32_t*)_ptr, CdlModeSpeed);
	if (CdReadSync(0, 0) < 0){}

    Fudgebundle *fdg = new Fudgebundle((uint8_t*)_ptr);
    blue = fdg->fudgebundle_get_texture(fdg_hash("blue"));
    green = fdg->fudgebundle_get_texture(fdg_hash("green"));
    orange = fdg->fudgebundle_get_texture(fdg_hash("orange"));
    purple = fdg->fudgebundle_get_texture(fdg_hash("purple"));
    red = fdg->fudgebundle_get_texture(fdg_hash("red"));
    turqoise = fdg->fudgebundle_get_texture(fdg_hash("turqoise"));
    yellow = fdg->fudgebundle_get_texture(fdg_hash("yellow"));


     for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            array[i][j] = new GameObject(FIELD_X+(j*CELL_SIZE),FIELD_Y+(i*CELL_SIZE),0);
            Sprite* sprite = new Sprite(SPRITE_TYPE_TEXTURED);
            sprite->tex = blue;
            sprite->Width = blue->width;
            sprite->Height = blue->height;
            array[i][j]->addComponent(sprite);
        }
    }

     for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            scene1->addGameObject(array[i][j]);
        }
     }

    psbw_load_scene(scene1);
}

void game_loop() {

}
