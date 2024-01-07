#include "psbw/Controller.h"
#include "ps1/registers.h"

static void delay_microsec(int time) {
	time *= (F_CPU + 1000000) / 2000000;

	__asm__ volatile(
		".set noreorder;"
		"bgtz  %0, .;"
		"addiu %0, -1;"
		".set reorder;"
		: "=r"(time) : "r"(time)
	);
}

void ctrl_init(void) {
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

static bool ctrl_wait_acknowledge(int timeout) {
	// Controllers and memory cards will acknowledge bytes received by sending
	// short pulses over the DSR line, which will be forwarded by the serial
	// interface to the interrupt controller. This is not guaranteed to happen
	// (it will not if e.g. no device is connected), so we have to implement a
	// timeout to avoid waiting forever in such cases.
	for (; timeout > 0; timeout -= 10) {
		if (IRQ_STAT & (1 << IRQ_SIO0)) {
			// Reset the interrupt controller and serial interface's flags to
			// ensure the interrupt can be triggered again.
			IRQ_STAT     = ~(1 << IRQ_SIO0);
			SIO_CTRL(0) |= SIO_CTRL_ACKNOWLEDGE;

			return true;
		}

		delay_microsec(10);
	}

	return false;
}

typedef enum {
	CMD_INIT_PRESSURE   = '@', // Initialize DualShock pressure sensors (config)
	CMD_POLL            = 'B', // Read controller state
	CMD_CONFIG_MODE     = 'C', // Enter or exit configuration mode
	CMD_SET_ANALOG      = 'D', // Set analog mode/LED state (config)
	CMD_GET_ANALOG      = 'E', // Get analog mode/LED state (config)
	CMD_GET_MOTOR_INFO  = 'F', // Get information about a motor (config)
	CMD_GET_MOTOR_LIST  = 'G', // Get list of all motors (config)
	CMD_GET_MOTOR_STATE = 'H', // Get current state of vibration motors (config)
	CMD_GET_MODE        = 'L', // Get list of all supported modes (config)
	CMD_REQUEST_CONFIG  = 'M', // Configure poll request format (config)
	CMD_RESPONSE_CONFIG = 'O', // Configure poll response format (config)
	CMD_CARD_READ       = 'R', // Read 128-byte memory card sector
	CMD_CARD_IDENTIFY   = 'S', // Retrieve memory card size information
	CMD_CARD_WRITE      = 'W'  // Write 128-byte memory card sector
} DeviceCommand;

#define DTR_DELAY   60
#define DSR_TIMEOUT 120

static void ctrl_port_select(int port) {
	// Set or clear the bit that controls which set of controller and memory
	// card ports is going to have its DTR (port select) signal asserted. The
	// actual serial bus is shared between all ports, however devices will not
	// process packets if DTR is not asserted on the port they are plugged into.
	if (port)
		SIO_CTRL(0) |= SIO_CTRL_CS_PORT_2;
	else
		SIO_CTRL(0) &= ~SIO_CTRL_CS_PORT_2;
}

static uint8_t ctrl_exchange_byte(uint8_t value) {
	// Wait until the interface is ready to accept a byte to send, then wait for
	// it to finish receiving the byte sent by the device.
	while (!(SIO_STAT(0) & SIO_STAT_TX_NOT_FULL))
		__asm__ volatile("");

	SIO_DATA(0) = value;

	while (!(SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY))
		__asm__ volatile("");

	return SIO_DATA(0);
}

static int ctrl_exchange_packet(
	const uint8_t *request, uint8_t *response,
	int reqLength, int maxRespLength
) {
	// Reset the interrupt flag and assert the DTR signal to tell the controller
	// or memory card that we're about to send a packet. Devices may take some
	// time to prepare for incoming bytes so we need a small delay here.
	IRQ_STAT     = ~(1 << IRQ_SIO0);
	SIO_CTRL(0) |= SIO_CTRL_DTR | SIO_CTRL_ACKNOWLEDGE;
	delay_microsec(DTR_DELAY);

	// Send the address byte and wait for the device to respond with a pulse on
	// the DSR line. If no response is received assume no device is connected,
	// otherwise make sure the serial interface's data buffer is empty to
	// prepare for the actual packet transfer.
	SIO_DATA(0) = 0x01;

	if (!ctrl_wait_acknowledge(DSR_TIMEOUT))
		return 0;
	while (SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY)
		SIO_DATA(0);

	int respLength = 0;

	// Send and receive the packet simultaneously one byte at a time, padding it
	// with zeroes if the packet we are receiving is longer than the data being
	// sent.
	while (respLength < maxRespLength) {
		if (reqLength > 0) {
			*(response++) = ctrl_exchange_byte(*(request++));
			reqLength--;
		} else {
			*(response++) = ctrl_exchange_byte(0);
		}

		respLength++;

		// The device will keep sending DSR pulses as long as there is more data
		// to transfer. If no more pulses are received, terminate the transfer.
		if (!ctrl_wait_acknowledge(DSR_TIMEOUT))
			break;
	}

	// Release DSR, allowing the device to go idle.
	delay_microsec(DTR_DELAY);
	SIO_CTRL(0) &= ~SIO_CTRL_DTR;

	return respLength;
}

Controller::Controller(ControllerPort controllerPort) {
    port = controllerPort;
}

int8_t Controller::GetButton(ControllerButton button) {

    uint8_t request[4], response[8];	

	request[0] = CMD_POLL; // Command
	request[1] = 0x00;     // Multitap address
	request[2] = 0x00;     // Rumble motor control 1
	request[3] = 0x00;     // Rumble motor control 2

    ctrl_port_select(port);
    int respLength = ctrl_exchange_packet(
		request, response, sizeof(request), sizeof(response)
	);

    if(respLength < 4) {
        // No controller is connected
        return -1;
    }

    // bytes 2 and 3 contain a bitfield of all the button states (inverted)
    uint16_t buttons = (response[2] | (response[3] << 8)) ^ 0xffff;

    // return button state
    return (buttons & button) ? 1 : 0;

}