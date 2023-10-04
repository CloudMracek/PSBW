#include <psbw/sio.h>

int main(int argc, const char **argv) {
	initSIO(SIO_BAUD_115200);
	printSIO("hello\n");
	for(;;) {
		printSIO("hello\n");
	}
	return 0;
}