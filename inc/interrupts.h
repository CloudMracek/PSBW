#pragma once

#include <ps1/system.h>

void interrupt_init();

void interrupt_install_callback(IRQChannel channel, void (*cb)(void));
