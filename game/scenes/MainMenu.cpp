#include "MainMenu.h"

#include <stdlib.h>
#include <stdio.h>

#include <psbw/Sound.h>
#include <psbw/Manager.h>

#include "Psxris.h"

// You have your constructor which needs to exist so it calls the base Scene class
MainMenu::MainMenu(char *name) : Scene(name) {}


// The destructor is called when you load another scene and you HAVE to clean up your mess in here
// We have 2 MB of RAM and that's NOTALOTTA...
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

// This is the scene setup function. It is called during loadtime of the scene (When you see the loading screen as a player)
// Its purpose is for you to get references for the assets in the fudgebundle you need and prepare your gameobjects and components
void MainMenu::sceneSetup()
{

    // Like this we can set a background image from fudgebundle
    Scene::setBackground("menubg");

    // You can play CDDA tracks. All you need to do is put your mp3s or whatevers into assets/music and you can then index them
    // alphabetically (starting from two because no.1 is the data track.)
    // So this plays second track and tells it to loop (second argument)
    soundPlayCdda(2,1);

    // If you have a custom font you can load it in like this but it's got some requirements. I suggest just using the font you can find in the assets folder
    pixelFontTexture = Scene::getTexture("font");
    pixelFont = new Font(pixelFontTexture);


    // And you can create gameobjects and components like this. I won't explain it any further. I'm lazy and it's pretty simple.
    // Level select control
    levelSelect = new GameObject(115, 102, 0);

    levelSelectBgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    levelSelectBgSprite->Width = 90;
    levelSelectBgSprite->Height = 20;
    levelSelectBgSprite->Color = {255, 255, 255};
    // Okay I will explain some things. the zIndex is opposite to normal CSS behavior. Meaning 0 is on top of 1.
    levelSelectBgSprite->zIndex = 2;
    levelSelect->addComponent(levelSelectBgSprite);

    levelSelectFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    // You can position the sprite component relatively to its parent GameObject, cool!
    levelSelectFgSprite->relPos = {3, 3, 0};
    levelSelectFgSprite->Width = 84;
    levelSelectFgSprite->Height = 14;
    levelSelectFgSprite->Color = {0, 0, 0};
    levelSelectFgSprite->zIndex = 1;
    levelSelect->addComponent(levelSelectFgSprite);

    levelSelectBuf = (char *)malloc(50);
    levelSelectText = new Text();
    levelSelectText->relPos = {40, 6, 0};
    levelSelectText->setFont(pixelFont);
    levelSelectText->text = levelSelectBuf;
    levelSelectText->zIndex = 0;
    sprintf(levelSelectBuf, "0");
    levelSelect->addComponent(levelSelectText);

    levelSelectLeftText = new Text();
    levelSelectLeftText->relPos = {6, 6, 0};
    levelSelectLeftText->setFont(pixelFont);
    levelSelectLeftText->text = "<";
    levelSelectLeftText->zIndex = 0;
    levelSelect->addComponent(levelSelectLeftText);

    levelSelectRightText = new Text();
    levelSelectRightText->relPos = {80, 6, 0};
    levelSelectRightText->setFont(pixelFont);
    levelSelectRightText->text = ">";
    levelSelectRightText->zIndex = 0;
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
    startButtonBgSprite->zIndex = 2;
    startButton->addComponent(startButtonBgSprite);

    startButtonFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    startButtonFgSprite->relPos = {3, 3, 0};
    startButtonFgSprite->Width = 84;
    startButtonFgSprite->Height = 14;
    startButtonFgSprite->Color = {0, 0, 0};
    startButtonFgSprite->zIndex = 1;
    startButton->addComponent(startButtonFgSprite);

    startButtonText = new Text();
    startButtonText->relPos = {31, 6, 0};
    startButtonText->setFont(pixelFont);
    startButtonText->text = "START";
    startButtonText->zIndex = 0;
    startButton->addComponent(startButtonText);

    currentlyPlayingLabel = new GameObject(5,229,0);
    currentlyPlayingLabelText = new Text();
    currentlyPlayingLabelText->setFont(pixelFont);
    currentlyPlayingLabelText->text = "RAMESES B - THERE FOR YOU [NCS RELEASE]";
    currentlyPlayingLabelText->zIndex = 0;
    currentlyPlayingLabel->addComponent(currentlyPlayingLabelText);


    // Don't forget to add your gameobjects to the scene at the end!
    // You can't remove them though. I was lazy. Not even components.
    Scene::addGameObject(levelSelect);
    Scene::addGameObject(startButton);
    Scene::addGameObject(currentlyPlayingLabel);

    // You can initialize your controller 
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

    // You have access to GetButton or GetButtonDown which handles previous state so it fires only once.
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
            // And here you can just load a new scene like that.
            Psxris *psxris = new Psxris("\\GAME.FDG", currentLevel);
            psbw_load_scene(psxris);
            // THIS RETURN IS VERY IMPORTANT. I'm an uggo and by this point this code shouldn't even exist because we
            // deleted the instance of MainMenu running this code in psbw_load_scene()
            // I don't care ¯\_(ツ)_/¯
            return;
        }
    }
}
