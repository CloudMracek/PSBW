#include "psbw/Text.h"
#include "psbw/font.h"


void Text::execute(GameObject* parent) {
    printString(parent->position.x, parent->position.y, text);
}