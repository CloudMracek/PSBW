#include "psbw/Text.h"

#include "psbw/Font.h"

void Text::setFont(Font* font) {
    _fnt = font;
}

void Text::execute(GameObject* parent) {
    _fnt->printString(parent->position.x, parent->position.y, text);
}