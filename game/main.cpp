#include "game/game.h"

#include <psbw/Manager.h>

#include "scenes/MainMenu.h"

void game_setup() {
    MainMenu *mainMenu = new MainMenu("\\BUNDLE.FDG");
    psbw_load_scene(mainMenu);
}