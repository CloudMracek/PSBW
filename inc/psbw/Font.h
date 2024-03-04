#pragma once
#include "psbw/Texture.h"

class Font {
    public:
        Font(Texture *texture);
        void printString(int x, int y, char *str, int zIndex);
    private:
        Texture *_tex;
};

