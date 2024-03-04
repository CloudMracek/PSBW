#include "psbw/Sio.h"

#include <ps1/registers.h>

static void printCharacter(char ch) {
	while (
		(SIO_STAT(1) & (SIO_STAT_TX_NOT_FULL | SIO_STAT_CTS)) == SIO_STAT_CTS
	)
		__asm__ volatile("");

	if (SIO_STAT(1) & SIO_STAT_CTS)
		SIO_DATA(1) = ch;
}

void sio_init(int baudrate) {
	SIO_CTRL(1) = SIO_CTRL_RESET;

	SIO_MODE(1) = SIO_MODE_BAUD_DIV16 | SIO_MODE_DATA_8 | SIO_MODE_STOP_1;
	SIO_BAUD(1) = (F_CPU / 16) / 115200;
	SIO_CTRL(1) = SIO_CTRL_TX_ENABLE | SIO_CTRL_RX_ENABLE | SIO_CTRL_RTS;
}

void sio_print(const char* str) {
	for (; *str; str++)
		printCharacter(*str);
}