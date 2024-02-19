#pragma once

#include <ps1/system.h>

#ifdef __cplusplus
extern "C" {
#endif

void interrupt_init();

void interrupt_install_callback(IRQChannel channel, void (*cb)(void));

#ifdef __cplusplus
}
#endif