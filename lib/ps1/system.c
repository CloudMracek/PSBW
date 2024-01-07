/*
 * ps1-bare-metal - (C) 2023 spicyjpeg
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include "ps1/cop0gte.h"
#include "ps1/registers.h"
#include "ps1/system.h"

#define BIOS_ENTRY_POINT ((VoidFunction)   0xbfc00000)
#define BIOS_API_TABLE   ((VoidFunction *) 0x80000200)
#define BIOS_BP_VECTOR   ((uint32_t *)     0x80000040)
#define BIOS_EXC_VECTOR  ((uint32_t *)     0x80000080)

/* Internal state */

static uint32_t     _savedBreakpointVector[4];
static uint32_t     _savedExceptionVector[4];
static VoidFunction _flushCache = 0;
static Thread       _mainThread;

ArgFunction interruptHandler     = 0;
void        *interruptHandlerArg = 0;

Thread *currentThread = &_mainThread;
Thread *nextThread    = &_mainThread;

/* Exception handler setup */

void _exceptionVector(void);

void installExceptionHandler(void) {
	// Clear all pending IRQ flags and prevent the interrupt controller from
	// generating further IRQs.
	IRQ_MASK = 0;
	IRQ_STAT = 0;
	DMA_DPCR = 0;
	DMA_DICR = DMA_DICR_CH_STAT_BITMASK;

	// Disable interrupts and the GTE at the COP0 side.
	cop0_setSR(COP0_SR_CU0);

	// Grab a direct pointer to the BIOS function to flush the instruction
	// cache. This is the only function that must always run from the BIOS ROM
	// as it temporarily disables main RAM.
	_flushCache = BIOS_API_TABLE[0x44];

	// Overwrite the default breakpoint and exception handlers placed into RAM
	// by the BIOS with a function that will jump to our custom handler.
	__builtin_memcpy(_savedBreakpointVector, BIOS_BP_VECTOR,  16);
	__builtin_memcpy(_savedExceptionVector,  BIOS_EXC_VECTOR, 16);
	__builtin_memcpy(BIOS_BP_VECTOR,  &_exceptionVector, 16);
	__builtin_memcpy(BIOS_EXC_VECTOR, &_exceptionVector, 16);
	_flushCache();

	DMA_DPCR = 0x0bbbbbbb;
	DMA_DICR = DMA_DICR_IRQ_ENABLE;

	// Ensure interrupts and the GTE are enabled at the COP0 side.
	cop0_setSR(COP0_SR_IEc | COP0_SR_Im2 | COP0_SR_CU0 | COP0_SR_CU2);
}

void uninstallExceptionHandler(void) {
	// Clear all pending IRQ flags and prevent the interrupt controller from
	// generating further IRQs.
	IRQ_MASK = 0;
	IRQ_STAT = 0;
	DMA_DPCR = 0;
	DMA_DICR = DMA_DICR_CH_STAT_BITMASK;

	// Disable interrupts and the GTE at the COP0 side.
	cop0_setSR(COP0_SR_CU0);

	// Restore the original BIOS breakpoint and exception handlers.
	__builtin_memcpy(BIOS_BP_VECTOR,  _savedBreakpointVector, 16);
	__builtin_memcpy(BIOS_EXC_VECTOR, _savedExceptionVector,  16);
	_flushCache();
}

void setInterruptHandler(ArgFunction func, void *arg) {
	disableInterrupts();

	interruptHandler    = func;
	interruptHandlerArg = arg;
	atomic_signal_fence(memory_order_release);
}

void flushCache(void) {
	//if (!_flushCache)
		//_flushCache = BIOS_API_TABLE[0x44];

	bool enable = disableInterrupts();

	_flushCache();
	if (enable)
		enableInterrupts();
}

void softReset(void) {
	disableInterrupts();
	BIOS_ENTRY_POINT();
	__builtin_unreachable();
}

/* IRQ acknowledgement */

bool acknowledgeInterrupt(IRQChannel irq) {
	if (IRQ_STAT & (1 << irq)) {
		IRQ_STAT = ~(1 << irq);
		return true;
	}

	return false;
}

bool waitForInterrupt(IRQChannel irq, int timeout) {
	for (; timeout > 0; timeout -= 10) {
		if (acknowledgeInterrupt(irq))
			return true;

		delayMicroseconds(10);
	}

	return false;
}

bool waitForDMATransfer(DMAChannel dma, int timeout) {
	for (; timeout > 0; timeout -= 10) {
		if (!(DMA_CHCR(dma) & DMA_CHCR_ENABLE))
			return true;

		delayMicroseconds(10);
	}

	return false;
}

/* Thread switching */

void switchThread(Thread *thread) {
	if (!thread)
		thread = &_mainThread;

	nextThread = thread;
	atomic_signal_fence(memory_order_release);
}
