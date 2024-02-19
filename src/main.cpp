
#include <stdio.h>

#include "game/game.h"

#include "ps1/system.h"
#include "draw.h"


#include "psbw/Sio.h"
#include "psbw/Controller.h"
#include "psbw/Sound.h"

extern "C" {
#include "cdrom.h"
#include "vsync.h"
#include "interrupts.h"
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