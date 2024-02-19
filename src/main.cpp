
#include <stdio.h>
#include "ps1/system.h"
#include "game/game.h"
#include "psbw/draw.h"


#include "psbw/sio.h"
#include "psbw/Controller.h"
#include "psbw/Sound.h"

extern "C" {
#include "psbw/cdrom.h"
#include "psbw/vsync.h"
#include "psbw/interrupts.h"
}


void main() {
	interrupt_init();
	sio_init(SIO_BAUD_115200);
	vsync_init();
	draw_init();
	ctrl_init();
	spu_init();
	CdInit();
	game_setup();

	for(;;) {
		ctrl_update();
		game_loop();
		draw_update();
	}
}