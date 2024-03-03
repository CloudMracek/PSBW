#include "game/game.h"

#include <psbw/Manager.h>

#include "scenes/Test3D.h"

void game_setup() {
    Test3D *test = new Test3D("\\MENU.FDG");
    psbw_load_scene(test);
}