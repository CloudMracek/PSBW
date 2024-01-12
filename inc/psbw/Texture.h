#pragma once
#include <stdint.h>


/**
 * \class Texture
 * \brief Add this component to your Sprite class to render a sprite
 */
class Texture {
    public:
	    uint8_t  u, v;
	    uint16_t width, height;
	    uint16_t page;
};