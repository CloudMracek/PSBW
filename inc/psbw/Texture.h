#pragma once
#include <stdint.h>

class Texture {
    public:
	    uint8_t  u, v;
	    uint16_t width, height;
	    uint16_t page;
};