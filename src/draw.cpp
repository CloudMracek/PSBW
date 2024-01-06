#include <ps1/gpucmd.h>
#include <ps1/registers.h>

#include "draw.h"
#include "Sprite.h"
#include "GameObject.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


GameObject* obj;
Sprite* spr;


// Private util functions

static void gpu_gp0_wait_ready(void) {
	while (!(GPU_GP1 & GP1_STAT_CMD_READY))
		__asm__ volatile("");
}

void gpu_setup(GP1VideoMode mode, int width, int height) {

	obj = new GameObject(100,100,100);
	spr = new Sprite(SPRITE_TYPE_FLAT_COLOR);

	spr->Color.x = 0;
	spr->Color.y = 255;
	spr->Color.z = 0;

	obj->components[0] = spr;

    // Origin of framebuffer based on if PAL or NTSC
    int x = 0x760;
    int y = (mode = GP1_MODE_PAL) ? 0xa3 : 0x88;

    // We need to do some timing magic to actually achieve our desired resolution
	GP1HorizontalRes horizontalRes = GP1_HRES_320;
	GP1VerticalRes   verticalRes   = GP1_VRES_256;

    int offsetX = (width  * gp1_clockMultiplierH(horizontalRes)) / 2;
	int offsetY = (height / gp1_clockDividerV(verticalRes))      / 2;

    GPU_GP1 = gp1_resetGPU();
	GPU_GP1 = gp1_fbRangeH(x - offsetX, x + offsetX);
	GPU_GP1 = gp1_fbRangeV(y - offsetY, y + offsetY);
	GPU_GP1 = gp1_fbMode(
		horizontalRes, verticalRes, mode, false, GP1_COLOR_16BPP
	);

}

static void gpu_wait_vsync(void) {
	while (!(IRQ_STAT & (1 << IRQ_VSYNC)))
		__asm__ volatile("");

	IRQ_STAT = ~(1 << IRQ_VSYNC);
}

// Engine's API

void draw_init() {
    if ((GPU_GP1 & GP1_STAT_MODE_BITMASK) == GP1_STAT_MODE_PAL) {
		gpu_setup(GP1_MODE_PAL, SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		gpu_setup(GP1_MODE_NTSC, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	GPU_GP1 = gp1_dispBlank(false);
}

int goRight = 1;

bool currentBuffer = false;
void draw_update() {
	int frameX = currentBuffer ? SCREEN_WIDTH : 0;
	int frameY = 0;

	currentBuffer = !currentBuffer;
	gpu_gp0_wait_ready();

	gpu_gp0_wait_ready();
	GPU_GP0 = gp0_texpage(0, true, false);
	GPU_GP0 = gp0_fbOffset1(frameX, frameY);
	GPU_GP0 = gp0_fbOffset2(
		frameX + SCREEN_WIDTH - 1, frameY + SCREEN_HEIGHT - 2
	);
	GPU_GP0 = gp0_fbOrigin(frameX, frameY);

	gpu_gp0_wait_ready();
	GPU_GP0 = gp0_rgb(64, 64, 64) | gp0_vramFill();
	GPU_GP0 = gp0_xy(frameX, frameY);
	GPU_GP0 = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);

	gpu_gp0_wait_ready();

	obj->execute();



	if(obj->position.x+spr->Width >= SCREEN_WIDTH && goRight) {
		goRight = 0;
	}

	if(!goRight && obj->position.x <= 0) {
		goRight = 1;
	}

	if(goRight) {
		obj->position.x++;
	}
	else {
		obj->position.x--;
	}

	gpu_gp0_wait_ready();
	gpu_wait_vsync();
	GPU_GP1 = gp1_fbOffset(frameX, frameY);
}


