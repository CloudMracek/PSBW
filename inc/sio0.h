#pragma once
#include <stdint.h>

void sio0_init(void);
uint8_t sio0_wait_acknowledge(int timeout);
void sio0_port_select(int port);
uint8_t sio0_exchange_byte(uint8_t value, bool wait);