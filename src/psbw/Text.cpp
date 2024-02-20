#include "psbw/Text.h"

#include "psbw/Font.h"

void Text::setFont(Font* font) {
    _fnt = font;
}

void Text::execute(GameObject* parent) {
    _fnt->printString(parent->position.x+Component::relPos.x, parent->position.y+Component::relPos.y, text);
}