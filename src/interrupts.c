#include "psbw/interrupts.h"

#include <ps1/registers.h>

void (*cdCallback)(void);
void (*vsyncCallback)(void);

static void _interruptHandler() {
    if (acknowledgeInterrupt(IRQ_CDROM)) {
        cdCallback();
    }
    if(acknowledgeInterrupt(IRQ_VSYNC)) {
        vsyncCallback();
    }
    // This is ugly but so are deadlines
    enableInterrupts();
}

void interrupt_init() {
    installExceptionHandler();
    setInterruptHandler(_interruptHandler, ((void *)0));
	enableInterrupts();
}

void interrupt_install_callback(IRQChannel channel, void (*cb)(void)) {
    switch (channel)
    {
    case IRQ_CDROM:
        cdCallback = cb;
        IRQ_MASK = 1 << IRQ_CDROM;
        break;
    
    case IRQ_VSYNC:
        vsyncCallback = cb;
        IRQ_MASK = 1 << IRQ_VSYNC;
        break;
    default:
        break;
    }
}