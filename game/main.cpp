#include <psbw/sio.h>
#include <psbw/manager.h>
#include <stdint.h>

#include <psbw/Scene.h>
#include <psbw/GameObject.h>
#include <psbw/Sprite.h>
#include <psbw/Texture.h>
#include <psbw/Controller.h>
#include <psbw/Sound.h>
#include "psbw/fudgebundle.h"
#include "psbw/font.h"
#include "ps1/system.h"

extern "C"
{
#include <psbw/filesystem.h>
#include <stdint.h>
#include "stdlib.h"
}

#include <vendor/printf.h>
#include <stdio.h>

#include "game/game.h"

#include "pieces.h"

Scene *scene1;

GameObject *fieldBackground, *fieldBorder;
Sprite *spriteFbg, *spriteFborder;

Texture *blue, *green, *orange, *purple, *red, *turqoise, *yellow;

Controller *controller1;

FDG_HASH_ENTRY *entry;

CdlFILE file;

#define FIELD_X 110
#define FIELD_Y 20

#define FIELD_BORDER_THICKNESS 3

#define FIELD_ROWS 20
#define FIELD_COLS 10
#define CELL_SIZE 10

Sprite *renderArray[FIELD_ROWS][FIELD_COLS];
BlockColor gameArray[FIELD_ROWS + 4][FIELD_COLS];

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

    for (int i = 0; i < FIELD_ROWS + 4; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            gameArray[i][j] = Empty;
        }
    }

    sound_play_cdda(2, 1);
    psbw_load_scene(scene1);
}

void render_piece(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
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
        return;
    }

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

void project_piece_into_game_array(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
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
        return;
    }

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            BlockColor blockColor = (*piece)[row][col];
            if (blockColor == Empty)
                continue;
            gameArray[row + y][col + x] = blockColor;
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
}

bool checkCollision(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
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
        return NULL;
    }

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

int8_t blockX = 3;
int8_t blockY = 0;
uint8_t blockType = randint(0, 6);
uint8_t fallTimer = 0;
uint8_t fallDelay = 25;

uint8_t currentDelay = 25;

uint8_t currentRotation = 0;

uint8_t buttonLeftTimeout, buttonRightTimeout = 0;
uint8_t buttonLeftTimer, buttonRightTimer = 0;

bool rotatePrevious = false;
bool buttonLeftFast, buttonRightFast = false;

void move_left()
{
    if (!checkCollision(blockX - 1, blockY, blockType, currentRotation))
    {
        blockX--;
    }
}

void move_right()
{
    if (!checkCollision(blockX + 1, blockY, blockType, currentRotation))
    {
        blockX++;
    }
}

void check_full_lines()
{
    int i, j, k;

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
            if (currentDelay > 5)
                currentDelay = currentDelay - 2;
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
}

void game_loop()
{

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

    if (controller1->GetButton(Up))
    {
        if (!rotatePrevious)
        {
            rotatePrevious = true;
            if (!checkCollision(blockX, blockY, blockType, (currentRotation + 1) % 4))
            {
                currentRotation = (currentRotation + 1) % 4;
            }
        }
    }
    else
    {
        rotatePrevious = false;
    }

    if (controller1->GetButton(Down))
    {
        fallDelay = 5;
    }
    else
    {
        fallDelay = currentDelay;
    }

    fallTimer++;
    if (fallTimer >= fallDelay)
    {
        fallTimer = 0;
        if (checkCollision(blockX, blockY + 1, blockType, currentRotation))
        {
            project_piece_into_game_array(blockX, blockY, blockType, currentRotation);
            check_full_lines();
            blockY = 0;
            blockX = 3;
            blockType = randint(0, 6);
        }
        else
        {
            blockY++;
        }
    }

    clear_render_array();
    render_game_array();
    render_piece(blockX, blockY, blockType, currentRotation);
}
