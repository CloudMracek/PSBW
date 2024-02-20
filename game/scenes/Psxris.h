#pragma once

#include <psbw/Scene.h>

#include <psbw/GameObject.h>
#include <psbw/Sprite.h>
#include <psbw/Text.h>
#include <psbw/Controller.h>
#include <psbw/Sound.h>

#define FIELD_X 110
#define FIELD_Y 20

#define FIELD_ROWS 20
#define FIELD_COLS 10
#define CELL_SIZE 10

typedef enum {
    Empty       = 0,
    Blue        = 1,
    Green       = 2,
    Orange      = 3,
    Purple      = 4,
    Red         = 5,
    Turqoise    = 6,
    Yellow      = 7
} BlockColor;


class Psxris : public Scene
{
public:
    Psxris(char *name, uint8_t startLevel);
    ~Psxris();

    void sceneSetup() override;
    void sceneLoop() override;

private:

    char *scoreBuf;
    char *levelBuf;

    Controller *controller1;
    bool buttonLeftFast, buttonRightFast = false;
    uint8_t buttonLeftTimeout, buttonRightTimeout = 0;
    uint8_t buttonLeftTimer, buttonRightTimer = 0;

    Texture *pixelFontTexture;
    Font *pixelFont;

    Texture *blue;
    Texture *green;
    Texture *orange;
    Texture *purple;
    Texture *red;
    Texture *turqoise;
    Texture *yellow;

    Sound *gameOverSound;
    Sound *placeSound;

    GameObject *objScore;
    Text *scoreText;

    GameObject *objLevel;
    Text* levelText;

    GameObject *objNextLabel;
    Text *nextLabelText;

    GameObject *objGameOver;
    Text *gameOverText;

    GameObject *currentlyPlayingLabel;
    Text* currentlyPlayingLabelText;

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
    unsigned int currentScore = 0;

    uint8_t linesCleared = 0;

    int8_t blockType = 0, nextBlockType = 0;

    bool gameOver = false;
    
    void startGame();

    void setupArrays();
    void cleanupArrays();
    void clearRenderArray();
    void clearGameArray();

    bool checkFallCollision(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation);
    bool checkRotationCollision(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation);

    void moveLeft();
    void moveRight();

    Texture *getBlockTexture(BlockColor blockColor);

    void renderGameArray();
    void renderPiece(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation);
    void renderPreviewPiece(uint8_t blockType, uint8_t rotation);

    void projectPieceIntoGameArray(int8_t x, int8_t y, uint8_t blockType, uint8_t rotation);
    void checkFullLines();
};
