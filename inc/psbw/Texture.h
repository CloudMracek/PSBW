#pragma once
#include <cstdint>

class Texture {
    public:
	    uint8_t  u, v;
	    uint16_t width, height;
	    uint16_t page;
};