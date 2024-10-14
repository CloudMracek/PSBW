#include "psbw/Controller.h"

#include <string.h>
#include <stdio.h>

#include "ps1/registers.h"
#include "ps1/system.h"

#include "sio0.h"



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



static int ctrl_exchange_packet(
	const uint8_t *request, uint8_t *response, int reqLength, int maxRespLength
) {
	// Reset the interrupt flag and assert the DTR signal to tell the controller
	// or memory card that we're about to send a packet. Devices may take some
	// time to prepare for incoming bytes so we need a small delay here.
	IRQ_STAT     = ~(1 << IRQ_SIO0);
	SIO_CTRL(0) |= SIO_CTRL_DTR | SIO_CTRL_ACKNOWLEDGE;
	delayMicroseconds(DTR_DELAY);

	int respLength = 0;

	// Send the address byte and wait for the device to respond with a pulse on
	// the DSR line. If no response is received assume no device is connected,
	// otherwise make sure the serial interface's data buffer is empty to
	// prepare for the actual packet transfer.
	SIO_DATA(0) = 0x01;

	if (sio0_wait_acknowledge(DSR_TIMEOUT)) {
		while (SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY)
			SIO_DATA(0);

		// Send and receive the packet simultaneously one byte at a time,
		// padding it with zeroes if the packet we are receiving is longer than
		// the data being sent.
		while (respLength < maxRespLength) {
			if (reqLength > 0) {
				*(response++) = sio0_exchange_byte(*(request++), false);
				reqLength--;
			} else {
				*(response++) = sio0_exchange_byte(0, false);
			}

			respLength++;

			// The device will keep sending DSR pulses as long as there is more
			// data to transfer. If no more pulses are received, terminate the
			// transfer.
			if (!sio0_wait_acknowledge(DSR_TIMEOUT))
				break;
		}
	}

	// Release DSR, allowing the device to go idle.
	delayMicroseconds(DTR_DELAY);
	SIO_CTRL(0) &= ~SIO_CTRL_DTR;

	return respLength;
}

uint8_t ctrlState[2][8];

uint8_t ctrlPrevState[2][8];

bool ctrlConnected[2];

void ctrl_update() {
    uint8_t request[4], response[8];

	request[0] = CMD_POLL; // Command
	request[1] = 0x00;     // Multitap address
	request[2] = 0x00;     // Rumble motor control 1
	request[3] = 0x00;     // Rumble motor control 2

	memcpy(&ctrlPrevState[CONTROLLER_PORT_1], &ctrlState[CONTROLLER_PORT_1], 8);
    sio0_port_select(CONTROLLER_PORT_1);
    int respLength = ctrl_exchange_packet(
		request, ctrlState[CONTROLLER_PORT_1], sizeof(request), sizeof(response)
	);

	if(respLength < 4) {
		ctrlConnected[CONTROLLER_PORT_1] = false;
	}
	else {
		ctrlConnected[CONTROLLER_PORT_1] = true;
	}

	memcpy(&ctrlPrevState[CONTROLLER_PORT_2], &ctrlState[CONTROLLER_PORT_2], 8);
    sio0_port_select(CONTROLLER_PORT_2);
    respLength = ctrl_exchange_packet(
		request, ctrlState[CONTROLLER_PORT_2], sizeof(request), sizeof(response)
	);

	if(respLength < 4) {
		ctrlConnected[CONTROLLER_PORT_2] = false;
	}
	else {
		ctrlConnected[CONTROLLER_PORT_2] = true;
	}
}

Controller::Controller(ControllerPort controllerPort) {
    port = controllerPort;
}

bool Controller::IsConnected() {
	return ctrlConnected[port];
}

bool Controller::GetButton(ControllerButton button) {
    if(!IsConnected()) {
        return false;
    }
    // bytes 2 and 3 contain a bitfield of all the button states (inverted)
    uint16_t buttons = (ctrlState[port][2] | (ctrlState[port][3] << 8)) ^ 0xffff;
    // return button state
    return (buttons & button) ? true : false;
}

bool Controller::GetButtonDown(ControllerButton button) {
    if(!IsConnected()) {
        return false;
    }
    // bytes 2 and 3 contain a bitfield of all the button states (inverted)
    uint16_t buttons = (ctrlState[port][2] | (ctrlState[port][3] << 8)) ^ 0xffff;
	uint16_t oldButtons = (ctrlPrevState[port][2] | (ctrlPrevState[port][3] << 8)) ^ 0xffff;
    // return button state
    if(buttons & button && !(oldButtons & button)) {
		return true;
	}
	else {
		return false;
	}
}
