#include <ps1/registers.h>
#include <ps1/system.h>

#include <stdio.h>

void sio0_init(void) {
	// Reset the serial interface, initialize it with the settings used by
	// controllers and memory cards (250000bps, 8 data bits) and configure it to
	// send a signal to the interrupt controller whenever the DSR input is
	// pulsed (see below).
	SIO_CTRL(0) = SIO_CTRL_RESET;

	SIO_MODE(0) = SIO_MODE_BAUD_DIV1 | SIO_MODE_DATA_8;
	SIO_BAUD(0) = F_CPU / 250000;
	SIO_CTRL(0) =
		SIO_CTRL_TX_ENABLE | SIO_CTRL_RX_ENABLE | SIO_CTRL_DSR_IRQ_ENABLE;
	
}

bool sio0_wait_acknowledge(int timeout) {
	// Controllers and memory cards will acknowledge bytes received by sending
	// short pulses over the DSR line, which will be forwarded by the serial
	// interface to the interrupt controller. This is not guaranteed to happen
	// (it will not if e.g. no device is connected), so we have to implement a
	// timeout to avoid waiting forever in such cases.
	if (waitForInterrupt(IRQ_SIO0, timeout)) {
		SIO_CTRL(0) |= SIO_CTRL_ACKNOWLEDGE;
		return true;
	}
	
	return false;
}

void sio0_port_select(int port) {
	// Set or clear the bit that controls which set of controller and memory
	// card ports is going to have its DTR (port select) signal asserted. The
	// actual serial bus is shared between all ports, however devices will not
	// process packets if DTR is not asserted on the port they are plugged into.
	if (port)
		SIO_CTRL(0) |= SIO_CTRL_CS_PORT_2;
	else
		SIO_CTRL(0) &= ~SIO_CTRL_CS_PORT_2;
}

uint8_t sio0_exchange_byte(uint8_t value, bool wait) {
	// Wait until the interface is ready to accept a byte to send, then wait for
	// it to finish receiving the byte sent by the device.
	while (!(SIO_STAT(0) & SIO_STAT_TX_NOT_FULL))
		asm("");
	
	//if(wait)
	//printf("SENDING: %x\n", value);
	SIO_DATA(0) = value;

	while (!(SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY))
		asm("");

	uint8_t response = SIO_DATA(0);
	//if(wait)
	//printf("RESPONSE: %x\n", response);

	if(wait) {
		if (!sio0_wait_acknowledge(1200))
		{
			printf("TIMEOUT!\n");
			return 0;
		}
	}

	return response;
}