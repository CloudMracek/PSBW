#include "game/game.h"

#include <psbw/Manager.h>
#include "scenes/MainMenu.h"

/* This file is the main entry point for the engine and probably the only
    place you want to touch this mess...
*/

void game_setup() {
    // All you need to do is create an instance of your scene and load it using the function in <psbw/Manager.h>
    MainMenu* mainMenu = new MainMenu("\\MENU.FDG");
    psbw_load_scene(mainMenu);
}