#include <psbw/Sio.h>
#include <psbw/Manager.h>
#include <stdint.h>

#include <psbw/Scene.h>
#include <psbw/GameObject.h>
#include <psbw/Sprite.h>
#include <psbw/Texture.h>
#include <psbw/Controller.h>
#include <psbw/Sound.h>
#include <psbw/Text.h>
#include "psbw/Fudgebundle.h"
#include "psbw/Font.h"
#include "ps1/system.h"

extern "C"
{
#include <filesystem.h>
#include <stdint.h>
#include "stdlib.h"
}

#include <stdio.h>

#include "game/game.h"

#include "pieces.h"
#include "draw.h"

Scene *scene1, *mainMenu;

GameObject *fieldBackground, *fieldBorder;
Sprite *spriteFbg, *spriteFborder;

Sprite *levelSelectBgSprite;
Sprite *startButtonBgSprite;

Texture *blue, *green, *orange, *purple, *red, *turqoise, *yellow, *font;
Text *gameOverText;

Sound *gameOverSound, *placeSound;

Controller *controller1;

FDG_HASH_ENTRY *entry;

CdlFILE file;

bool gameOver = false;
bool mainMenuButton = false;

char *score;
char *level;
char *levelSelect;

#define FIELD_X 110
#define FIELD_Y 20

#define FIELD_BORDER_THICKNESS 3

#define FIELD_ROWS 20
#define FIELD_COLS 10
#define CELL_SIZE 10

Sprite *renderArray[FIELD_ROWS][FIELD_COLS];
Sprite *previewRenderArray[4][4];

BlockColor gameArray[FIELD_ROWS + 4][FIELD_COLS];

int8_t blockX = 3;
int8_t blockY = 0;
int8_t fallTimer = 0;
uint8_t fallDelay;

uint8_t currentLevel = 0;
uint8_t currentDelay;
uint8_t currentRotation = 0;

int8_t blockType = 0, nextBlockType = 0;

uint8_t buttonLeftTimeout, buttonRightTimeout = 0;
uint8_t buttonLeftTimer, buttonRightTimer = 0;

bool rotatePrevious = false;
bool buttonLeftFast, buttonRightFast = false;

unsigned int currentScore = 0;
uint8_t linesCleared = 0;

uint8_t fallDelays[2][21] = {
    // For 50 FPS
    {45, 41, 38, 35, 32, 29, 24, 19, 15, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2},
    // For 60 FPS
    {54, 49, 45, 41, 37, 33, 28, 22, 17, 11, 10, 9, 8, 7, 6, 6, 5, 5, 4, 4, 3}};

uint32_t scoringTable[9][4] = {
    // Points for 1 line, 2 lines, 3 lines, 4 lines
    {100, 400, 900, 2000},   // Level 0
    {100, 400, 900, 2000},   // Level 1
    {200, 800, 1800, 4000},  // Level 2
    {200, 800, 1800, 4000},  // Level 3
    {300, 1200, 2700, 6000}, // Level 4
    {300, 1200, 2700, 6000}, // Level 5
    {400, 1600, 3600, 8000}, // Level 6
    {400, 1600, 3600, 8000}, // Level 7
    {500, 2000, 4500, 10000} // Level 8+
};

bool inMainMenu = true;

void start_game()
{
    currentDelay = fallDelays[draw_get_graphics_mode()][currentLevel];

    update_random_seed();
    blockType = randint(0, 6);
    nextBlockType = randint(0, 6);
    currentScore = 0;

    sound_play_cdda(itob(randint(2, 6)), 1);
    inMainMenu = false;
    gameOver = false;
    gameOverText->text = "";
    sprintf(score, "SCORE: 0");
    sprintf(level, "LEVEL: %d", currentLevel);

    psbw_load_scene(scene1);
}

void game_setup()
{

    controller1 = new Controller(CONTROLLER_PORT_1);

    scene1 = new Scene();

    CdSearchFile(&file, "\\BUNDLE.FDG");
    size_t len = (file.size + 2047) & 0xfffff800;
    void *_ptr = malloc(len);
    CdControl(CdlSetloc, &(file.pos), 0);
    CdRead(len / 2048, (uint32_t *)_ptr, CdlModeSpeed);
    if (CdReadSync(0, 0) < 0)
    {
    }

    Fudgebundle *fdg = new Fudgebundle((uint8_t *)_ptr);
    blue = fdg->fudgebundle_get_texture(fdg_hash("blue"));
    green = fdg->fudgebundle_get_texture(fdg_hash("green"));
    orange = fdg->fudgebundle_get_texture(fdg_hash("orange"));
    purple = fdg->fudgebundle_get_texture(fdg_hash("purple"));
    red = fdg->fudgebundle_get_texture(fdg_hash("red"));
    turqoise = fdg->fudgebundle_get_texture(fdg_hash("turqoise"));
    yellow = fdg->fudgebundle_get_texture(fdg_hash("yellow"));

    font = fdg->fudgebundle_get_texture(fdg_hash("font"));

    scene1->backgroundImage = fdg->fudgebundle_get_background(fdg_hash("gamebg"));

    gameOverSound = fdg->fudgebundle_get_sound(fdg_hash("gameover"));
    placeSound = fdg->fudgebundle_get_sound(fdg_hash("place"));

    setFont(font);

    spriteFborder = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    spriteFborder->Color = {255, 255, 255};
    spriteFborder->Width = (FIELD_COLS * CELL_SIZE) + FIELD_BORDER_THICKNESS * 2;
    spriteFborder->Height = (FIELD_ROWS * CELL_SIZE) + FIELD_BORDER_THICKNESS * 2;

    fieldBorder = new GameObject(FIELD_X - FIELD_BORDER_THICKNESS, FIELD_Y - FIELD_BORDER_THICKNESS, 0);
    fieldBorder->addComponent(spriteFborder);
    scene1->addGameObject(fieldBorder);

    spriteFbg = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    spriteFbg->Color = {0, 0, 0};
    spriteFbg->Width = (FIELD_COLS * CELL_SIZE);
    spriteFbg->Height = (FIELD_ROWS * CELL_SIZE);

    fieldBackground = new GameObject(FIELD_X, FIELD_Y, 0);
    fieldBackground->addComponent(spriteFbg);
    scene1->addGameObject(fieldBackground);

    for (int i = 0; i < FIELD_ROWS; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            GameObject *object = new GameObject(FIELD_X + (j * CELL_SIZE), FIELD_Y + (i * CELL_SIZE), 0);
            Sprite *sprite = new Sprite(SPRITE_TYPE_TEXTURED);
            sprite->tex = nullptr;
            sprite->Width = blue->width;
            sprite->Height = blue->height;
            object->addComponent(sprite);
            renderArray[i][j] = sprite;
            scene1->addGameObject(object);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            GameObject *object = new GameObject(242 + (j * CELL_SIZE), 35 + (i * CELL_SIZE), 0);
            Sprite *sprite = new Sprite(SPRITE_TYPE_TEXTURED);
            sprite->tex = nullptr;
            sprite->Width = blue->width;
            sprite->Height = blue->height;
            object->addComponent(sprite);
            previewRenderArray[i][j] = sprite;
            scene1->addGameObject(object);
        }
    }

    for (int i = 0; i < FIELD_ROWS + 4; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            gameArray[i][j] = Empty;
        }
    }

    score = (char *)malloc(50);
    level = (char *)malloc(50);

    GameObject *objScore = new GameObject(227, 103, 0);
    Text *scoreText = new Text();
    scoreText->text = score;
    objScore->addComponent(scoreText);

    GameObject *objLevel = new GameObject(227, 114, 0);
    Text *levelText = new Text();
    levelText->text = level;
    objLevel->addComponent(levelText);

    GameObject *objNext = new GameObject(242, 22, 0);
    Text *nextText = new Text();
    nextText->text = "NEXT:";
    objNext->addComponent(nextText);

    sprintf(score, "SCORE: 0");
    sprintf(level, "LEVEL: %d", currentLevel);

    scene1->addGameObject(objScore);
    scene1->addGameObject(objLevel);
    scene1->addGameObject(objNext);

    GameObject *gameOverObj = new GameObject(150, 30, 0);
    gameOverText = new Text();

    gameOverText->text = "";
    gameOverObj->addComponent(gameOverText);
    scene1->addGameObject(gameOverObj);

    sound_play_cdda(2, 1);

    mainMenu = new Scene();
    mainMenu->backgroundImage = fdg->fudgebundle_get_background(fdg_hash("menubg"));

    GameObject *levelSelectBg = new GameObject(115, 102, 0);
    levelSelectBgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    levelSelectBgSprite->Width = 90;
    levelSelectBgSprite->Height = 20;
    levelSelectBgSprite->Color = {255, 255, 255};
    levelSelectBg->addComponent(levelSelectBgSprite);

    GameObject *levelSelectFg = new GameObject(118, 105, 0);
    Sprite *levelSelectFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    levelSelectFgSprite->Width = 84;
    levelSelectFgSprite->Height = 14;
    levelSelectFgSprite->Color = {0, 0, 0};
    levelSelectFg->addComponent(levelSelectFgSprite);

    GameObject *startButtonBg = new GameObject(115, 135, 0);
    startButtonBgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    startButtonBgSprite->Width = 90;
    startButtonBgSprite->Height = 20;
    startButtonBgSprite->Color = {255, 255, 255};
    startButtonBg->addComponent(startButtonBgSprite);

    GameObject *startButtonFg = new GameObject(118, 138, 0);
    Sprite *startButtonFgSprite = new Sprite(SPRITE_TYPE_FLAT_COLOR);
    startButtonFgSprite->Width = 84;
    startButtonFgSprite->Height = 14;
    startButtonFgSprite->Color = {0, 0, 0};
    startButtonFg->addComponent(startButtonFgSprite);

    levelSelect = (char *)malloc(50);
    GameObject *levelSelectText = new GameObject(155, 108, 0);
    Text *levelSelectTextCmp = new Text();
    levelSelectTextCmp->text = levelSelect;
    sprintf(levelSelect, "0");
    levelSelectText->addComponent(levelSelectTextCmp);

    GameObject *startButtonText = new GameObject(146, 141, 0);
    Text *startButtonTextCmp = new Text();
    startButtonTextCmp->text = "START";
    startButtonText->addComponent(startButtonTextCmp);

    GameObject *levelSelectLeft = new GameObject(121, 108, 0);
    Text *levelSelectLeftText = new Text();
    levelSelectLeftText->text = "<";
    levelSelectLeft->addComponent(levelSelectLeftText);

    GameObject *levelSelectRight = new GameObject(195, 108, 0);
    Text *levelSelectRightText = new Text();
    levelSelectRightText->text = ">";
    levelSelectRight->addComponent(levelSelectRightText);

    GameObject *levelSelectLabel = new GameObject(116, 92, 0);
    Text *levelSelectLabelText = new Text();
    levelSelectLabelText->text = "LEVEL:";
    levelSelectLabel->addComponent(levelSelectLabelText);

    mainMenu->addGameObject(levelSelectBg);
    mainMenu->addGameObject(levelSelectFg);
    mainMenu->addGameObject(startButtonBg);
    mainMenu->addGameObject(startButtonFg);
    mainMenu->addGameObject(levelSelectText);
    mainMenu->addGameObject(startButtonText);
    mainMenu->addGameObject(levelSelectLeft);
    mainMenu->addGameObject(levelSelectRight);
    mainMenu->addGameObject(levelSelectLabel);

    if (!inMainMenu)
    {
        psbw_load_scene(scene1);
    }
    else
    {
        psbw_load_scene(mainMenu);
    }
}

BlockColor (*get_block_arrray(uint8_t blockType, uint8_t rotation))[4][4]
{
    BlockColor(*piece)[4][4]; // Define a pointer to a 4x4  renderArray of BlockColor

    switch (blockType)
    {
    case 0: // I Block
        piece = &I_block[rotation];
        break;

    case 1: // J Block
        piece = &J_block[rotation];
        break;

    case 2: // L Block
        piece = &L_block[rotation];
        break;

    case 3: // O Block
        piece = &O_block[rotation];
        break;

    case 4: // S Block
        piece = &S_block[rotation];
        break;

    case 5: // T Block
        piece = &T_block[rotation];
        break;

    case 6: // Z Block
        piece = &Z_block[rotation];
        break;

    default:
        return NULL; // Returning NULL as a default case since the function return type is not void
    }

    return piece;
}

void render_piece(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
{
    BlockColor(*piece)[4][4] = get_block_arrray(blockType, rotation);
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {

            int newY = y + row - 4;
            if (newY < 0)
                continue;

            Texture *color;
            // swap
            BlockColor blockColor = (*piece)[row][col];

            switch (blockColor)
            {
            case Empty:
                color = nullptr;
                break;
            case Blue:
                color = blue;
                break;

            case Green:
                color = green;
                break;

            case Orange:
                color = orange;
                break;

            case Purple:
                color = purple;
                break;

            case Red:
                color = red;
                break;

            case Turqoise:
                color = turqoise;
                break;

            case Yellow:
                color = yellow;
                break;

            default:
                // Handle any other cases or errors
                break;
            }

            if (newY > 19 || col + x > 9 || col + x < 0 || blockColor == Empty)
                continue;

            renderArray[newY][col + x]->tex = color;
        }
    }
}

void render_game_array()
{
    for (int row = 0; row < FIELD_ROWS + 4; row++)
    {
        for (int col = 0; col < FIELD_COLS; col++)
        {
            Texture *color;
            BlockColor blockColor = gameArray[row][col];

            switch (blockColor)
            {
            case Empty:
                color = nullptr;
                break;
            case Blue:
                color = blue;
                break;

            case Green:
                color = green;
                break;

            case Orange:
                color = orange;
                break;

            case Purple:
                color = purple;
                break;

            case Red:
                color = red;
                break;

            case Turqoise:
                color = turqoise;
                break;

            case Yellow:
                color = yellow;
                break;

            default:
                // Handle any other cases or errors
                break;
            }

            if (row - 4 < 0)
                continue;

            renderArray[row - 4][col]->tex = color;
        }
    }
}

void render_preview_piece(uint8_t blockType, uint8_t rotation)
{
    uint8_t x = 0, y = 0;
    BlockColor(*piece)[4][4] = get_block_arrray(blockType, rotation);
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {

            int newY = y + row;

            Texture *color;
            // swap
            BlockColor blockColor = (*piece)[row][col];

            switch (blockColor)
            {
            case Empty:
                color = nullptr;
                break;
            case Blue:
                color = blue;
                break;

            case Green:
                color = green;
                break;

            case Orange:
                color = orange;
                break;

            case Purple:
                color = purple;
                break;

            case Red:
                color = red;
                break;

            case Turqoise:
                color = turqoise;
                break;

            case Yellow:
                color = yellow;
                break;

            default:
                // Handle any other cases or errors
                break;
            }

            if (newY > 19 || col + x > 9 || col + x < 0 || blockColor == Empty)
                continue;

            previewRenderArray[newY][col + x]->tex = color;
        }
    }
}

void project_piece_into_game_array(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
{
    BlockColor(*piece)[4][4] = get_block_arrray(blockType, rotation);

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            BlockColor blockColor = (*piece)[row][col];
            if (blockColor == Empty)
                continue;
            if ((row + y) < 4)
            {
                gameOver = true;
            }
            gameArray[row + y][col + x] = blockColor;
        }
    }
}

void clear_game_array()
{
    for (int i = 0; i < FIELD_ROWS+4; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            gameArray[i][j] = Empty;
        }
    }
}

void clear_render_array()
{
    for (int i = 0; i < FIELD_ROWS; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            renderArray[i][j]->tex = nullptr;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            previewRenderArray[i][j]->tex = nullptr;
        }
    }
}

bool check_fall_collision(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
{
    BlockColor(*piece)[4][4] = get_block_arrray(blockType, rotation);

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            BlockColor blockColor = (*piece)[row][col];
            if (blockColor == Empty)
                continue;
            if (y + row > FIELD_ROWS + 3)
                return true;
            if (x + col > FIELD_COLS - 1 || x + col < 0)
                return true;
            BlockColor gameArrrayBlockColor = gameArray[y + row][x + col];
            if (gameArrrayBlockColor != Empty)
                return true;
        }
    }
    return false;
}

bool check_rotation_position(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
{
    BlockColor(*piece)[4][4] = get_block_arrray(blockType, rotation);

    bool colliding = false;
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            BlockColor blockColor = (*piece)[row][col];
            if (blockColor == Empty)
                continue;
            if (y + row > FIELD_ROWS + 3)
                colliding = true;
            if (x + col > FIELD_COLS - 1 || x + col < 0)
                colliding = true;
            BlockColor gameArrrayBlockColor = gameArray[y + row][x + col];
            if (gameArrrayBlockColor != Empty)
                colliding = true;
        }
    }

    if (colliding && x < 5)
    {
        if (!check_fall_collision(x + 1, y, blockType, rotation))
        {
            blockX++;
            return false;
        }
        else if (!check_fall_collision(x + 2, y, blockType, rotation))
        {
            blockX += 2;
            return false;
        }
    }
    else if (colliding && x >= 5)
    {
        if (!check_fall_collision(x - 1, y, blockType, rotation))
        {
            blockX--;
            return false;
        }
        else if (!check_fall_collision(x - 2, y, blockType, rotation))
        {
            blockX -= 2;
            return false;
        }
    }
    return colliding;
}

void move_left()
{
    if (!check_fall_collision(blockX - 1, blockY, blockType, currentRotation))
    {
        blockX--;
    }
}

void move_right()
{
    if (!check_fall_collision(blockX + 1, blockY, blockType, currentRotation))
    {
        blockX++;
    }
}

void check_full_lines()
{
    int i, j, k;
    int foundFull = 0;

    for (i = 0; i < FIELD_ROWS + 4; i++)
    {
        int full_row = 1;
        for (j = 0; j < FIELD_COLS; j++)
        {
            if (gameArray[i][j] == Empty)
            {
                full_row = 0;
                break;
            }
        }
        if (full_row)
        {
            foundFull++;
            // Clear the full row
            for (j = 0; j < FIELD_COLS; j++)
            {
                gameArray[i][j] = Empty;
            }
            // Move rows down
            for (k = i; k > 0; k--)
            {
                for (j = 0; j < FIELD_COLS; j++)
                {
                    gameArray[k][j] = gameArray[k - 1][j];
                }
            }

            for (int j = 0; j < FIELD_COLS; j++)
            {
                gameArray[0][j] = Empty;
            }
        }
    }

    if (foundFull > 0)
    {
        if (currentLevel >= 8)
        {   
            unsigned int scoreToAdd = currentScore + scoringTable[8][foundFull - 1];
            currentScore += scoreToAdd;
            sprintf(score, "SCORE: %u", currentScore);
        }
        else
        {
            unsigned int scoreToAdd = scoringTable[currentLevel][foundFull - 1];
            currentScore += scoreToAdd;
            sprintf(score, "SCORE: %u", currentScore);
        }
    }

    linesCleared += foundFull;
    if (linesCleared >= 10)
    {
        linesCleared = 0;
        if (currentLevel < 20)
        {
            currentLevel++;
            sprintf(level, "LEVEL: %u", currentLevel);
            currentDelay = fallDelays[draw_get_graphics_mode()][currentLevel];
        }
    }
}

void game_loop()
{

    if (inMainMenu)
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
                    sprintf(levelSelect, "%d", currentLevel);
                }
            }
            else if (controller1->GetButtonDown(Left))
            {
                if (currentLevel > 0)
                {
                    currentLevel--;
                    sprintf(levelSelect, "%d", currentLevel);
                }
            }
        }
        if (mainMenuButton)
        {
            if (controller1->GetButtonDown(X))
            {
                start_game();
            }
        }

        return;
    }

    if (gameOver)
    {
        sound_stop_cdda();
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 30; j++)
                draw_update();
            clear_render_array();
            for (int j = 0; j < 30; j++)
                draw_update();
            render_game_array();
        }
        clear_render_array();
        clear_game_array();
        gameOverSound->play();
        gameOverText->text = "GAME\nOVER\nPRESS\nX\nTO\nRETURN";
        draw_update();
        while (1)
        {
            ctrl_update();
            if (controller1->GetButton(X))
            {
                inMainMenu = true;
                sound_play_cdda(2,1);
                psbw_load_scene(mainMenu);
                return;
            }
        }
    }

    if (controller1->GetButton(Left))
    {
        if (buttonLeftTimer >= buttonLeftTimeout && !buttonLeftFast)
        {
            buttonLeftTimeout = 10;
            buttonLeftTimer = 0;
            buttonLeftFast = true;
            move_left();
        }
        else if (buttonLeftTimer >= buttonLeftTimeout && buttonLeftFast)
        {
            buttonLeftTimeout = 3;
            buttonLeftTimer = 0;
            move_left();
        }
        buttonLeftTimer++;
    }
    else
    {
        buttonLeftTimeout = 0;
        buttonLeftTimer = 0;
        buttonLeftFast = false;
    }

    if (controller1->GetButton(Right))
    {
        if (buttonRightTimer >= buttonRightTimeout && !buttonRightFast)
        {
            buttonRightTimeout = 10;
            buttonRightTimer = 0;
            buttonRightFast = true;
            move_right();
        }
        else if (buttonRightTimer >= buttonRightTimeout && buttonRightFast)
        {
            buttonRightTimeout = 3;
            buttonRightTimer = 0;
            move_right();
        }
        buttonRightTimer++;
    }
    else
    {
        buttonRightTimeout = 0;
        buttonRightTimer = 0;
        buttonRightFast = false;
    }

    fallDelay = currentDelay;
    if (controller1->GetButton(Down))
    {
        fallDelay = 5;
    }

    if (controller1->GetButtonDown(Up))
    {
        if (!check_rotation_position(blockX, blockY, blockType, (currentRotation + 1) % 4))
        {
            currentRotation = (currentRotation + 1) % 4;
            fallTimer -= 10;
        }
    }

    fallTimer++;
    if (fallTimer >= fallDelay)
    {
        fallTimer = 0;
        if (check_fall_collision(blockX, blockY + 1, blockType, currentRotation))
        {
            placeSound->play();
            project_piece_into_game_array(blockX, blockY, blockType, currentRotation);
            check_full_lines();
            blockY = 0;
            blockX = 3;
            blockType = nextBlockType;
            nextBlockType = randint(0, 6);
        }
        else
        {
            blockY++;
        }
    }

    clear_render_array();
    render_game_array();
    render_preview_piece(nextBlockType, 0);
    render_piece(blockX, blockY, blockType, currentRotation);
}
