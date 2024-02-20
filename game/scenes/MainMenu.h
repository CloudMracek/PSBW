#pragma once

#include <psbw/Scene.h>

#include <psbw/GameObject.h>
#include <psbw/Sprite.h>
#include <psbw/Text.h>
#include <psbw/Controller.h>

class MainMenu : public Scene {
    public:
        MainMenu(char* name);
        ~MainMenu();
        
        void sceneSetup() override;
        void sceneLoop() override;
    private:
        char* levelSelectBuf;

        bool mainMenuButton = false;
        uint8_t currentLevel = 0;

        Controller *controller1;

        Texture *pixelFontTexture;
        Font *pixelFont;


        GameObject *levelSelect;
        Text *levelSelectText;
        Sprite *levelSelectBgSprite;
        Sprite *levelSelectFgSprite;
        Text* levelSelectLeftText;
        Text* levelSelectRightText;
        Text* levelSelectLabelText;

        GameObject *startButton;
        Text *startButtonText;
        Sprite *startButtonBgSprite;
        Sprite *startButtonFgSprite;
        

        GameObject *currentlyPlayingLabel;
        Text* currentlyPlayingLabelText;

};