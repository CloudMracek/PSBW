#include "game/game.h"
#include "psbw/draw.h"
#include "psbw/sio.h"
#include "psbw/Controller.h"

void main() {
	sio_init(SIO_BAUD_115200);
	draw_init();
	ctrl_init();
	game_setup();

	for(;;) {
		game_loop();
		draw_update();
		for(int i = 0; i < 20000; i++);
	}
}