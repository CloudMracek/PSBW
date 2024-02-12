#include <stdint.h>
#include "psbw/vsync.h"
#include <ps1/registers.h>
#include <ps1/system.h>
#include "psbw/interrupts.h"

#define VSYNC_TIMEOUT	0x100000

static volatile uint32_t _vblank_counter, _last_vblank;
static volatile uint16_t _last_hblank;

static void (*_vsync_callback)(void)    = (void *) 0;


static void _vblank_handler(void) {
	_vblank_counter++;

	if (_vsync_callback)
		_vsync_callback();
}

static void _default_vsync_halt(void) {
	int counter = _vblank_counter;
	for (int i = VSYNC_TIMEOUT; i; i--) {
		if (counter != _vblank_counter)
			return;
	}
}

void vsync_init() {
    _vblank_counter = 0;
	_last_vblank    = 0;
	_last_hblank    = 0;
    GPU_GP1 = 0x02000000;
    interrupt_install_callback(IRQ_VSYNC, &_vblank_handler);
}

static void (*_vsync_halt_func)(void)   = &_default_vsync_halt;


int VSync(int mode) {
	uint16_t delta = (TIMER_VALUE(1) - _last_hblank) & 0xffff;
	if (mode == 1)
		return delta;
	if (mode < 0)
		return _vblank_counter;

	// Wait for the specified number of vertical blank events since the last
	// call to VSync() to occur (if mode >= 2) or just for a single vertical
	// blank (if mode = 0).
	uint32_t target = mode ? (_last_vblank + mode) : (_vblank_counter + 1);

	while (_vblank_counter < target) {
		uint32_t status = GPU_GP1;
		_vsync_halt_func();

		// If interlaced mode is enabled, wait until the GPU starts displaying
		// the next field.
		if (status & (1 << 22)) {
			while (!((GPU_GP1 ^ status) & (1 << 31)))
				__asm__ volatile("");
		}
	}

	_last_vblank = _vblank_counter;
	_last_hblank = TIMER_VALUE(1);

	return delta;
}