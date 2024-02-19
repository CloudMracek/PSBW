#include "MainMenu.h"

#include <stdlib.h>
#include <stdio.h>

#include <psbw/Sound.h>

MainMenu::MainMenu(char *name) : Scene(name) {}

MainMenu::~MainMenu()
{
    Scene::~Scene();
}

void MainMenu::sceneSetup()
{
    Scene::setBackground("menubg");

    soundPlayCdda(2,1);

    pixelFontTexture = Scene::getTexture("font");
    pixelFont = new Font(pixelFontTexture);

    levelSelectBg = new GameObject(115, 102, 0);
    levelSelectBgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    levelSelectBgSprite->Width = 90;
    levelSelectBgSprite->Height = 20;
    levelSelectBgSprite->Color = {255, 255, 255};
    levelSelectBg->addComponent(levelSelectBgSprite);

    levelSelectFg = new GameObject(118, 105, 0);
    levelSelectFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    levelSelectFgSprite->Width = 84;
    levelSelectFgSprite->Height = 14;
    levelSelectFgSprite->Color = {0, 0, 0};
    levelSelectFg->addComponent(levelSelectFgSprite);

    startButtonBg = new GameObject(115, 135, 0);
    startButtonBgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    startButtonBgSprite->Width = 90;
    startButtonBgSprite->Height = 20;
    startButtonBgSprite->Color = {255, 255, 255};
    startButtonBg->addComponent(startButtonBgSprite);

    startButtonFg = new GameObject(118, 138, 0);
    startButtonFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    startButtonFgSprite->Width = 84;
    startButtonFgSprite->Height = 14;
    startButtonFgSprite->Color = {0, 0, 0};
    startButtonFg->addComponent(startButtonFgSprite);

    levelSelectBuf = (char *)malloc(50);
    levelSelect = new GameObject(155, 108, 0);
    levelSelectText = new Text();
    levelSelectText->setFont(pixelFont);
    levelSelectText->text = levelSelectBuf;
    sprintf(levelSelectBuf, "0");
    levelSelect->addComponent(levelSelectText);

    startButton = new GameObject(146, 141, 0);
    startButtonText = new Text();
    startButtonText->setFont(pixelFont);
    startButtonText->text = "START";
    startButton->addComponent(startButtonText);

    levelSelectLeft = new GameObject(121, 108, 0);
    levelSelectLeftText = new Text();
    levelSelectLeftText->setFont(pixelFont);
    levelSelectLeftText->text = "<";
    levelSelectLeft->addComponent(levelSelectLeftText);

    levelSelectRight = new GameObject(195, 108, 0);
    levelSelectRightText = new Text();
    levelSelectRightText->setFont(pixelFont);
    levelSelectRightText->text = ">";
    levelSelectRight->addComponent(levelSelectRightText);

    levelSelectLabel = new GameObject(116, 92, 0);
    levelSelectLabelText = new Text();
    levelSelectLabelText->setFont(pixelFont);
    levelSelectLabelText->text = "LEVEL:";
    levelSelectLabel->addComponent(levelSelectLabelText);

    Scene::addGameObject(levelSelectBg);
    Scene::addGameObject(levelSelectFg);
    Scene::addGameObject(startButtonBg);
    Scene::addGameObject(startButtonFg);
    Scene::addGameObject(levelSelect);
    Scene::addGameObject(startButton);
    Scene::addGameObject(levelSelectLeft);
    Scene::addGameObject(levelSelectRight);
    Scene::addGameObject(levelSelectLabel);

    controller1 = new Controller(CONTROLLER_PORT_1);
}

void MainMenu::sceneLoop()
{
    if (mainMenuButton)
    {
        levelSelectBgSprite->Color = {255, 255, 255};
        startButtonBgSprite->Color = {255, 0, 0};
    }
    else
    {
        levelSelectBgSprite->Color = {255, 0, 0};
        startButtonBgSprite->Color = {255, 255, 255};
    }

    if (controller1->GetButtonDown(Up) || controller1->GetButtonDown(Down))
    {
        mainMenuButton = !mainMenuButton;
    }

    if (!mainMenuButton)
    {
        if (controller1->GetButtonDown(Right))
        {
            if (currentLevel < 20)
            {
                currentLevel++;
                sprintf(levelSelectBuf, "%d", currentLevel);
            }
        }
        else if (controller1->GetButtonDown(Left))
        {
            if (currentLevel > 0)
            {
                currentLevel--;
                sprintf(levelSelectBuf, "%d", currentLevel);
            }
        }
    }
    if (mainMenuButton)
    {
        if (controller1->GetButtonDown(X))
        {
            //start_game();
        }
    }
}
