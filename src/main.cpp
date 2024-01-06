#include "game/game.h"
#include "draw.h"
#include "psbw/sio.h"

void main() {
	sio_init(SIO_BAUD_115200);
	game_setup();
	draw_init();

	for(;;) {
		game_loop();
		draw_update();
		for(int i = 0; i < 20000; i++);
	}
}