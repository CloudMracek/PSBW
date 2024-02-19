#include "psbw/Text.h"

#include "psbw/Font.h"

void Text::execute(GameObject* parent) {
    printString(parent->position.x, parent->position.y, text);
}