
#include <stdio.h>
#include "ps1/system.h"
#include "game/game.h"
#include "psbw/draw.h"
#include "psbw/sio.h"
#include "psbw/Controller.h"
#include "psbw/Sound.h"

extern "C" {
#include "psbw/cdrom.h"
}


void main() {
	// Uncomment to install custom interrupt handler (see inc/ps1/system.h)
	//installExceptionHandler();

	sio_init(SIO_BAUD_115200);
	draw_init();
	ctrl_init();
	spu_init();
	CdInit();
	game_setup();

	for(;;) {
		game_loop();
		draw_update();
		for(int i = 0; i < 20000; i++);
	}
}