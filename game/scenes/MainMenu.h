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

        GameObject *levelSelectBg;
        Sprite *levelSelectBgSprite;

        GameObject *levelSelectFg;
        Sprite *levelSelectFgSprite;

        GameObject *startButtonFg;
        Sprite *startButtonFgSprite;

        GameObject *startButtonBg;
        Sprite *startButtonBgSprite;

        GameObject *levelSelect;
        Text *levelSelectText;

        GameObject *startButton;
        Text *startButtonText;

        GameObject *levelSelectLeft;
        Text* levelSelectLeftText;

        GameObject *levelSelectRight;
        Text* levelSelectRightText;

        GameObject *levelSelectLabel;
        Text* levelSelectLabelText;

};