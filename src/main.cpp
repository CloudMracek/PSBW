
#include <stdio.h>

#include "game/game.h"

#include "ps1/system.h"

#include "draw.h"
#include "cdrom.h"
#include "vsync.h"
#include "interrupts.h"

#include "psbw/Sio.h"
#include "sio0.h"
#include "psbw/Controller.h"
#include "psbw/Sound.h"

void main() {
	interrupt_init();
	sio_init(SIO_BAUD_115200);
	vsync_init();
	draw_init();
	sio0_init();
	spu_init();
	CdInit();
	game_setup();

	for(;;) {
		ctrl_update();
		draw_update(true);
	}
}