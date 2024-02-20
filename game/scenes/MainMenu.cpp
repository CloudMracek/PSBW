#include "MainMenu.h"

#include <stdlib.h>
#include <stdio.h>

#include <psbw/Sound.h>
#include <psbw/Manager.h>

#include "Psxris.h"

MainMenu::MainMenu(char *name) : Scene(name) {}

MainMenu::~MainMenu()
{
    free(levelSelectBuf);
    delete controller1;
    
    delete pixelFontTexture;
    delete pixelFont;


    delete startButton;
    delete startButtonBgSprite;
    delete startButtonFgSprite;
    delete startButtonText;

    delete levelSelect;
    delete levelSelectBgSprite;
    delete levelSelectFgSprite;
    delete levelSelectText;
    delete levelSelectLeftText;
    delete levelSelectRightText;
    delete levelSelectLabelText;



    delete currentlyPlayingLabel;
    delete currentlyPlayingLabelText;

    Scene::~Scene();
}

void MainMenu::sceneSetup()
{
    Scene::setBackground("menubg");

    soundPlayCdda(2,1);

    pixelFontTexture = Scene::getTexture("font");
    pixelFont = new Font(pixelFontTexture);


    // Level select control
    levelSelect = new GameObject(115, 102, 0);

    levelSelectBgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    levelSelectBgSprite->Width = 90;
    levelSelectBgSprite->Height = 20;
    levelSelectBgSprite->Color = {255, 255, 255};
    levelSelect->addComponent(levelSelectBgSprite);

    levelSelectFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    levelSelectFgSprite->relPos = {3, 3, 0};
    levelSelectFgSprite->Width = 84;
    levelSelectFgSprite->Height = 14;
    levelSelectFgSprite->Color = {0, 0, 0};
    levelSelect->addComponent(levelSelectFgSprite);

    levelSelectBuf = (char *)malloc(50);
    levelSelectText = new Text();
    levelSelectText->relPos = {40, 6, 0};
    levelSelectText->setFont(pixelFont);
    levelSelectText->text = levelSelectBuf;
    sprintf(levelSelectBuf, "0");
    levelSelect->addComponent(levelSelectText);

    levelSelectLeftText = new Text();
    levelSelectLeftText->relPos = {6, 6, 0};
    levelSelectLeftText->setFont(pixelFont);
    levelSelectLeftText->text = "<";
    levelSelect->addComponent(levelSelectLeftText);

    levelSelectRightText = new Text();
    levelSelectRightText->relPos = {80, 6, 0};
    levelSelectRightText->setFont(pixelFont);
    levelSelectRightText->text = ">";
    levelSelect->addComponent(levelSelectRightText);

    levelSelectLabelText = new Text();
    levelSelectLabelText->relPos = {1, -10, 0};
    levelSelectLabelText->setFont(pixelFont);
    levelSelectLabelText->text = "LEVEL:";
    levelSelect->addComponent(levelSelectLabelText);

    // Start button control
    startButton = new GameObject(115, 135, 0);
    startButtonBgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    startButtonBgSprite->Width = 90;
    startButtonBgSprite->Height = 20;
    startButtonBgSprite->Color = {255, 255, 255};
    startButton->addComponent(startButtonBgSprite);

    startButtonFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    startButtonFgSprite->relPos = {3, 3, 0};
    startButtonFgSprite->Width = 84;
    startButtonFgSprite->Height = 14;
    startButtonFgSprite->Color = {0, 0, 0};
    startButton->addComponent(startButtonFgSprite);

    startButtonText = new Text();
    startButtonText->relPos = {31, 6, 0};
    startButtonText->setFont(pixelFont);
    startButtonText->text = "START";
    startButton->addComponent(startButtonText);

    currentlyPlayingLabel = new GameObject(5,229,0);
    currentlyPlayingLabelText = new Text();
    currentlyPlayingLabelText->setFont(pixelFont);
    currentlyPlayingLabelText->text = "RAMESES B - THERE FOR YOU [NCS RELEASE]";
    currentlyPlayingLabel->addComponent(currentlyPlayingLabelText);

    Scene::addGameObject(levelSelect);
    Scene::addGameObject(startButton);
    Scene::addGameObject(currentlyPlayingLabel);

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
            Psxris *psxris = new Psxris("\\GAME.FDG", currentLevel);
            psbw_load_scene(psxris);
            return;
        }
    }
}
