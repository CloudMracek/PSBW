#include "Psxris.h"

#include <stdlib.h>
#include <stdio.h>

#include <ps1/system.h>
#include <draw.h>

#include "psbw/Manager.h"

#include "MainMenu.h"

// blue
#define _E Empty
#define _B Blue
#define _G Green
#define _O Orange
#define _P Purple
#define _R Red
#define _T Turqoise
#define _Y Yellow

#pragma region BLOCKDEF
BlockColor I_block[4][4][4] = {
    {
        {_E, _E, _E, _E},
        {_E, _E, _E, _E},
        {_B, _B, _B, _B},
        {_E, _E, _E, _E}
    },
    {
        {_E, _B, _E, _E},
        {_E, _B, _E, _E},
        {_E, _B, _E, _E},
        {_E, _B, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_B, _B, _B, _B},
        {_E, _E, _E, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _B, _E},
        {_E, _E, _B, _E},
        {_E, _E, _B, _E},
        {_E, _E, _B, _E}
    }
};

 BlockColor J_block[4][4][4] = {
    {
        {_E, _E, _E, _E},
        {_E, _G, _E, _E},
        {_E, _G, _G, _G},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_E, _E, _G, _E},
        {_E, _E, _G, _E},
        {_E, _G, _G, _E}
    },
    {
        {_E, _E, _E, _E},
        {_G, _G, _G, _E},
        {_E, _E, _G, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _G, _G, _E},
        {_E, _G, _E, _E},
        {_E, _G, _E, _E},
        {_E, _E, _E, _E}
    }
};

 BlockColor L_block[4][4][4] = {
    {
        {_E, _E, _E, _E},
        {_E, _E, _P, _E},
        {_P, _P, _P, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _P, _P, _E},
        {_E, _E, _P, _E},
        {_E, _E, _P, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_E, _E, _E, _E},
        {_P, _P, _P, _E},
        {_E, _E, _P, _E},
    },
    {
        {_E, _P, _E, _E},
        {_E, _P, _E, _E},
        {_E, _P, _P, _E},
        {_E, _E, _E, _E}
    }
};

 BlockColor O_block[4][4][4] = {
    {
        {_E, _E, _E, _E},
        {_E, _Y, _Y, _E},
        {_E, _Y, _Y, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_E, _Y, _Y, _E},
        {_E, _Y, _Y, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_E, _Y, _Y, _E},
        {_E, _Y, _Y, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_E, _Y, _Y, _E},
        {_E, _Y, _Y, _E},
        {_E, _E, _E, _E}
    }
};

 BlockColor S_block[4][4][4] = {
    {
        {_E, _E, _E, _E},
        {_E, _E, _T, _T},
        {_E, _T, _T, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _T, _E, _E},
        {_E, _T, _T, _E},
        {_E, _E, _T, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_E, _E, _T, _T},
        {_E, _T, _T, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _T, _E, _E},
        {_E, _T, _T, _E},
        {_E, _E, _T, _E},
        {_E, _E, _E, _E}
    }
};

 BlockColor T_block[4][4][4] = {
    {
        {_E, _E, _E, _E},
        {_E, _R, _E, _E},
        {_R, _R, _R, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _R, _E, _E},
        {_E, _R, _R, _E},
        {_E, _R, _E, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_R, _R, _R, _E},
        {_E, _R, _E, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _R, _E, _E},
        {_R, _R, _E, _E},
        {_E, _R, _E, _E},
        {_E, _E, _E, _E}
    }
};

 BlockColor Z_block[4][4][4] = {
    {
        {_E, _E, _E, _E},
        {_O, _O, _E, _E},
        {_E, _O, _O, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _O, _E},
        {_E, _O, _O, _E},
        {_E, _O, _E, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _E, _E},
        {_O, _O, _E, _E},
        {_E, _O, _O, _E},
        {_E, _E, _E, _E}
    },
    {
        {_E, _E, _O, _E},
        {_E, _O, _O, _E},
        {_E, _O, _E, _E},
        {_E, _E, _E, _E}
    }
};

BlockColor (*getBlockArray(uint8_t blockType, uint8_t rotation))[4][4]
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


#pragma endregion


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


Psxris::Psxris(char *name, uint8_t startLevel) : Scene(name) {
    currentLevel = startLevel;
}

Psxris::~Psxris()
{
    free(scoreBuf);
    free(levelBuf);

    delete controller1;
    delete pixelFontTexture;
    delete pixelFont;

    delete blue;
    delete green;
    delete orange;
    delete purple;
    delete red;
    delete turqoise;
    delete yellow;
    
    delete gameOverSound;
    delete placeSound;

    delete objScore;
    delete scoreText;
    
    delete objLevel;
    delete levelText;

    delete objNextLabel;
    delete nextLabelText;

    delete objGameOver;
    delete gameOverText;

    cleanupArrays();
    
    Scene::~Scene();
}

void Psxris::sceneSetup()
{
    Scene::setBackground("gamebg");

    pixelFontTexture = Scene::getTexture("font");
    pixelFont = new Font(pixelFontTexture);

    controller1 = new Controller(CONTROLLER_PORT_1);

    blue = Scene::getTexture("blue");
    green = Scene::getTexture("green");
    orange = Scene::getTexture("orange");
    purple = Scene::getTexture("purple");
    red = Scene::getTexture("red");
    turqoise = Scene::getTexture("turqoise");
    yellow = Scene::getTexture("yellow");

    gameOverSound = Scene::getSound("gameover");
    placeSound = Scene::getSound("place");

    setupArrays();

    scoreBuf = (char *)malloc(50);
    levelBuf = (char *)malloc(50);

    objScore = new GameObject(227, 103, 0);
    scoreText = new Text();
    scoreText->setFont(pixelFont);
    scoreText->text = scoreBuf;
    objScore->addComponent(scoreText);

    objLevel = new GameObject(227, 114, 0);
    levelText = new Text();
    levelText->setFont(pixelFont);
    levelText->text = levelBuf;
    objLevel->addComponent(levelText);

    objNextLabel = new GameObject(242, 22, 0);
    nextLabelText = new Text();
    nextLabelText->setFont(pixelFont);
    nextLabelText->text = "NEXT:";
    objNextLabel->addComponent(nextLabelText);

    objGameOver = new GameObject(150, 30, 0);
    gameOverText = new Text();
    gameOverText->setFont(pixelFont);
    gameOverText->text = "";
    objGameOver->addComponent(gameOverText);

    Scene::addGameObject(objScore);
    Scene::addGameObject(objLevel);
    Scene::addGameObject(objNextLabel);
    Scene::addGameObject(objGameOver);

    startGame();
}

void Psxris::setupArrays() {
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
            Scene::addGameObject(object);
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
            Scene::addGameObject(object);
        }
    }

    for (int i = 0; i < FIELD_ROWS + 4; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            gameArray[i][j] = Empty;
        }
    }

}

void Psxris::cleanupArrays() {
    for (int i = 0; i < FIELD_ROWS; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            // TODO: LEAKS MEMORY BECAUSE GAMEOBJECT IS NOT ACCOUNTED FOR
            free(renderArray[i][j]);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // TODO: SAME HERE
            free(previewRenderArray[i][j]);
            
        }
    }
}

Texture *Psxris::getBlockTexture(BlockColor blockColor) {
    switch (blockColor)
            {
            case Empty:
                return nullptr;
                break;
            case Blue:
                return blue;
                break;

            case Green:
                return green;
                break;

            case Orange:
                return orange;
                break;

            case Purple:
                return purple;
                break;

            case Red:
                return red;
                break;

            case Turqoise:
                return turqoise;
                break;

            case Yellow:
                return yellow;
                break;

            default:
                return nullptr;
                break;
            }
}

void Psxris::startGame() {

    currentDelay = fallDelays[draw_get_graphics_mode()][currentLevel];

    blockType = randint(0, 6);
    nextBlockType = randint(0, 6);

    soundPlayCdda(randint(2, 6), 1);
    gameOver = false;
    gameOverText->text = "";
    sprintf(scoreBuf, "SCORE: 0");
    sprintf(levelBuf, "LEVEL: %d", currentLevel);
}

void Psxris::clearGameArray()
{
    for (int i = 0; i < FIELD_ROWS+4; i++)
    {
        for (int j = 0; j < FIELD_COLS; j++)
        {
            gameArray[i][j] = Empty;
        }
    }
}

void Psxris::clearRenderArray()
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


bool Psxris::checkFallCollision(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
{
    BlockColor(*piece)[4][4] = getBlockArray(blockType, rotation);

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

bool Psxris::checkRotationCollision(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
{
    BlockColor(*piece)[4][4] = getBlockArray(blockType, rotation);

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
        if (!checkFallCollision(x + 1, y, blockType, rotation))
        {
            blockX++;
            return false;
        }
        else if (!checkFallCollision(x + 2, y, blockType, rotation))
        {
            blockX += 2;
            return false;
        }
    }
    else if (colliding && x >= 5)
    {
        if (!checkFallCollision(x - 1, y, blockType, rotation))
        {
            blockX--;
            return false;
        }
        else if (!checkFallCollision(x - 2, y, blockType, rotation))
        {
            blockX -= 2;
            return false;
        }
    }
    return colliding;
}

void Psxris::moveLeft()
{
    if (!checkFallCollision(blockX - 1, blockY, blockType, currentRotation))
    {
        blockX--;
    }
}

void Psxris::moveRight()
{
    if (!checkFallCollision(blockX + 1, blockY, blockType, currentRotation))
    {
        blockX++;
    }
}

void Psxris::projectPieceIntoGameArray(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation) 
{
    BlockColor(*piece)[4][4] = getBlockArray(blockType, rotation);

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            BlockColor blockColor = (*piece)[row][col];
            if (blockColor == Empty)
                continue;
            if ((row + y) < 4)
            {
                gameOverSound->play();
                gameOver = true;
                return;
            }
            gameArray[row + y][col + x] = blockColor;
        }
    }
}

void Psxris::renderGameArray()
{
    for (int row = 0; row < FIELD_ROWS + 4; row++)
    {
        for (int col = 0; col < FIELD_COLS; col++)
        {
            Texture *color;
            BlockColor blockColor = gameArray[row][col];

            color = getBlockTexture(blockColor);

            if (row - 4 < 0)
                continue;

            renderArray[row - 4][col]->tex = color;
        }
    }
}

void Psxris::renderPiece(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation)
{
    BlockColor(*piece)[4][4] = getBlockArray(blockType, rotation);
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

            color = getBlockTexture(blockColor);
            
            if (newY > 19 || col + x > 9 || col + x < 0 || blockColor == Empty)
                continue;

            renderArray[newY][col + x]->tex = color;
        }
    }
}

void Psxris::renderPreviewPiece(uint8_t blockType, uint8_t rotation)
{
    uint8_t x = 0, y = 0;
    BlockColor(*piece)[4][4] = getBlockArray(blockType, rotation);
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {

            int newY = y + row;

            Texture *color;
            // swap
            BlockColor blockColor = (*piece)[row][col];

            color = getBlockTexture(blockColor);

            if (newY > 19 || col + x > 9 || col + x < 0)
                continue;

            previewRenderArray[newY][col + x]->tex = color;
        }
    }
}

void Psxris::checkFullLines()
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
            sprintf(scoreBuf, "SCORE: %u", currentScore);
        }
        else
        {
            unsigned int scoreToAdd = scoringTable[currentLevel][foundFull - 1];
            currentScore += scoreToAdd;
            sprintf(scoreBuf, "SCORE: %u", currentScore);
        }
    }

    linesCleared += foundFull;
    if (linesCleared >= 10)
    {
        linesCleared = 0;
        if (currentLevel < 20)
        {
            currentLevel++;
            sprintf(levelBuf, "LEVEL: %u", currentLevel);
            currentDelay = fallDelays[draw_get_graphics_mode()][currentLevel];
        }
    }
}

void Psxris::sceneLoop()
{
   if (gameOver)
    {
        soundStopCdda();
        clearRenderArray();
        clearGameArray();
        gameOverText->text = "GAME\nOVER\nPRESS\nX\nTO\nRETURN";

        if (controller1->GetButton(X))
        {
            MainMenu *menu = new MainMenu("\\BUNDLE.FDG");
            psbw_load_scene(menu);
            return;
        }
        return;
    }

    if (controller1->GetButton(Left))
    {
        if (buttonLeftTimer >= buttonLeftTimeout && !buttonLeftFast)
        {
            buttonLeftTimeout = 10;
            buttonLeftTimer = 0;
            buttonLeftFast = true;
            moveLeft();
        }
        else if (buttonLeftTimer >= buttonLeftTimeout && buttonLeftFast)
        {
            buttonLeftTimeout = 3;
            buttonLeftTimer = 0;
            moveLeft();
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
            moveRight();
        }
        else if (buttonRightTimer >= buttonRightTimeout && buttonRightFast)
        {
            buttonRightTimeout = 3;
            buttonRightTimer = 0;
            moveRight();
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
        if (!checkRotationCollision(blockX, blockY, blockType, (currentRotation + 1) % 4))
        {
            currentRotation = (currentRotation + 1) % 4;
            fallTimer -= 10;
        }
    }

    fallTimer++;
    if (fallTimer >= fallDelay)
    {
        fallTimer = 0;
        if (checkFallCollision(blockX, blockY + 1, blockType, currentRotation))
        {
            placeSound->play();
            projectPieceIntoGameArray(blockX, blockY, blockType, currentRotation);
            checkFullLines();
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

    clearRenderArray();
    renderGameArray();
    renderPreviewPiece(nextBlockType, 0);
    renderPiece(blockX, blockY, blockType, currentRotation);
}
