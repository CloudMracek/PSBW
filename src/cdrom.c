#include "ps1/registers.h"
#include "ps1/system.h"
#include "psbw/interrupts.h"
#include "psbw/cdrom.h"

#define CD_ACK_TIMEOUT		0x100000
#define CD_SYNC_TIMEOUT		0x100000
#define MAX_RESULT_SIZE		32

typedef enum {
	CdlNoIntr		= 0,	// No pending interrupt
	CdlDataReady	= 1,	// INT1 (new sector or CD-DA report packet available)
	CdlComplete		= 2,	// INT2 ("complete" response received, blocking command has finished)
	CdlAcknowledge	= 3,	// INT3 ("acknowledge" response received, non-blocking command has finished or blocking command has started)
	CdlDataEnd		= 4,	// INT4 (end of track or end of disc reached)
	CdlDiskError	= 5		// INT5 (command error, read error or lid opened)
} CdlIntrResult;

typedef struct {
	uint8_t minute;	// Minutes (BCD)
	uint8_t second;	// Seconds (BCD)
	uint8_t sector;	// Sector or frame (BCD)
	uint8_t track;	// Track number
} CdlLOC;

typedef struct {
	uint8_t val0;	// CD to SPU L-to-L volume
	uint8_t val1;	// CD to SPU L-to-R volume
	uint8_t val2;	// CD to SPU R-to-R volume
	uint8_t val3;	// CD to SPU R-to-L volume
} CdlATV;

typedef enum {
	CdlNop			= 0x01,	// Updates the current CD-ROM status and resets the CdlStatShellOpen flag, without doing anything else.
	CdlSetloc		= 0x02,	// Sets the seek target location, but does not seek. Actual seeking begins upon issuing a seek or read command.
	CdlPlay			= 0x03,	// Begins CD-DA playback. Parameter specifies an optional track number to play (some emulators do not support it).
	CdlForward		= 0x04,	// Starts fast-forwarding (CD-DA only). Issue CdlPlay to stop fast-forwarding.
	CdlBackward		= 0x05,	// Starts rewinding (CD-DA only). Issue CdlPlay to stop rewinding.
	CdlReadN		= 0x06,	// Begins reading data sectors and/or playing XA-ADPCM with automatic retry. Used in conjunction with CdReadyCallback().
	CdlStandby		= 0x07,	// Starts the spindle motor if it was previously stopped.
	CdlStop			= 0x08,	// Stops playback or data reading and shuts down the spindle motor.
	CdlPause		= 0x09,	// Stops playback or data reading without stopping the spindle motor.
	CdlInit			= 0x0a,	// Initializes the CD-ROM controller and aborts any ongoing command.
	CdlMute			= 0x0b,	// Mutes the drive's audio output (both CD-DA and XA-ADPCM).
	CdlDemute		= 0x0c,	// Unmutes the drive's audio output (both CD-DA and XA-ADPCM).
	CdlSetfilter	= 0x0d,	// Configures the XA-ADPCM sector filter.
	CdlSetmode		= 0x0e,	// Sets the CD-ROM mode flags (see CdlModeFlags).
	CdlGetparam		= 0x0f,	// Returns the current CD-ROM mode flags and XA-ADPCM filter settings.
	CdlGetlocL		= 0x10,	// Returns the location, mode and XA subheader of the current data sector. Does not work on CD-DA sectors.
	CdlGetlocP		= 0x11,	// Returns the current physical CD location (using subchannel Q data).
	CdlSetsession	= 0x12,	// Attempts to seek to the specified session on a multi-session disc. Used by CdLoadSession().
	CdlGetTN		= 0x13,	// Returns the total number of tracks on the disc.
	CdlGetTD		= 0x14,	// Returns the starting location of the specified track number.
	CdlSeekL		= 0x15,	// Seeks (using data sector headers) to the position set by the last CdlSetloc command. Does not work on CD-DA sectors.
	CdlSeekP		= 0x16,	// Seeks (using subchannel Q data) to the position set by the last CdlSetloc command.
	CdlTest			= 0x19,	// Executes a test subcommand. Shall be issued using CdCommand() rather than CdControl().
	CdlGetID		= 0x1a,	// Identifies the disc type and returns its license string if any.
	CdlReadS		= 0x1b,	// Begins reading data sectors and/or playing XA-ADPCM in real-time (without automatic retry) mode.
	CdlReset		= 0x1c,	// Resets the CD-ROM controller (similar behavior to manually opening and closing the lid).
	CdlGetQ			= 0x1d,	// Reads up to 10 raw bytes of subchannel Q data directly from the disc's table of contents.
	CdlReadTOC		= 0x1e	// Forces reading of the disc's table of contents.
} CdlCommand;

typedef enum {
	CdlStatError		= 1 << 0,	// A command error has occurred. Set when an invalid command or parameters are sent.
	CdlStatStandby		= 1 << 1,	// Set whenever the spindle motor is powered on or spinning up.
	CdlStatSeekError	= 1 << 2,	// A seek error has occurred.
	CdlStatIdError		= 1 << 3,	// Disc has been rejected due to being unlicensed (on consoles without a modchip installed).
	CdlStatShellOpen	= 1 << 4,	// Lid is open or has been opened before. This flag is cleared by sending a CdlNop command.
	CdlStatRead			= 1 << 5,	// Drive is currently reading data and/or playing XA-ADPCM.
	CdlStatSeek			= 1 << 6,	// Drive is currently seeking.
	CdlStatPlay			= 1 << 7	// Drive is currently playing a CD-DA track.
} CdlStatFlag;

typedef enum {
	PARAM_1		= 1,
	PARAM_2		= 2,
	PARAM_3		= 3,
	STATUS		= 1 << 2,	// First byte of CdlAcknowledge response is status
	C_STATUS	= 1 << 3,	// First byte of CdlComplete response is status
	BLOCKING	= 1 << 4,	// Command triggers CdlComplete interrupt
	OPTIONAL	= 1 << 5,	// Parameter is optional for the command
	SETLOC		= 1 << 6	// Parameter shall be sent as a separate CdlSetloc
} CommandFlags;


static const uint8_t _command_flags[] = {
	0,
	STATUS,									// CdlNop
	STATUS | PARAM_3,						// CdlSetloc
	STATUS | OPTIONAL | PARAM_1,			// CdlPlay
	//STATUS | SETLOC,						// CdlPlay
	STATUS,									// CdlForward
	STATUS,									// CdlBackward
	STATUS | SETLOC,						// CdlReadN
	STATUS | C_STATUS | BLOCKING,			// CdlStandby
	STATUS | C_STATUS | BLOCKING,			// CdlStop
	STATUS | C_STATUS | BLOCKING,			// CdlPause
	STATUS | C_STATUS | BLOCKING,			// CdlInit
	STATUS,									// CdlMute
	STATUS,									// CdlDemute
	STATUS | PARAM_2,						// CdlSetfilter
	STATUS | PARAM_1,						// CdlSetmode
	STATUS,									// CdlGetparam
	0,										// CdlGetlocL
	0,										// CdlGetlocP
	STATUS | C_STATUS | BLOCKING | PARAM_1,	// CdlSetsession
	STATUS,									// CdlGetTN
	STATUS | PARAM_1,						// CdlGetTD
	STATUS | C_STATUS | BLOCKING | SETLOC,	// CdlSeekL
	STATUS | C_STATUS | BLOCKING | SETLOC,	// CdlSeekP
	0,
	0,
	PARAM_1,								// CdlTest
	STATUS | C_STATUS | BLOCKING,			// CdlGetID
	STATUS | SETLOC,						// CdlReadS
	STATUS,									// CdlReset
	STATUS | BLOCKING | PARAM_2,			// CdlGetQ
	STATUS | C_STATUS | BLOCKING			// CdlReadTOC
};

typedef void (*CdlCB)(CdlIntrResult, uint8_t *);

CdlCB _cd_override_callback;


static CdlCB _ready_callback = (CdlCB) 0;
static CdlCB _sync_callback  = (CdlCB) 0;
static CdlCB _pause_callback = (CdlCB) 0;


static uint8_t *_result_ptr;
static uint8_t _last_command, _last_mode;
static CdlLOC  _last_pos;

static volatile uint8_t _last_status, _last_irq, _last_error;
static volatile uint8_t _ack_pending, _sync_pending;

volatile int _cd_media_changed;

static void _update_status(uint8_t status) {
	uint8_t last = _last_status;
	_last_status = status;

	if (!(last & CdlStatError) && (status & CdlStatError))

	if (!(last & CdlStatShellOpen) && (status & CdlStatShellOpen)) {
		_cd_media_changed = 1;
	}
}

CdlIntrResult CdSync(int mode, uint8_t *result) {
	if (mode) {
		if (_sync_pending)
			return CdlNoIntr;

		if (result)
			*result = _last_status;
		if (_last_irq == CdlAcknowledge)
			return CdlComplete;

		return (CdlIntrResult)_last_irq;
	}

	for (int i = CD_SYNC_TIMEOUT; i; i--) {
		if (!_sync_pending)
			return CdSync(1, result); // :P
	}

	return (CdlIntrResult)-1;
}


int CdCommandF(CdlCommand cmd, const void *param, int length) {

	const uint8_t *_param = (const uint8_t *) param;

	_last_command = (uint8_t) cmd;
	_ack_pending  = 1;

	if (cmd <= CdlReadTOC) {
		if (_command_flags[cmd] & BLOCKING)
			_sync_pending = 1;

		// Keep track of the last mode and seek location set (so retries can be
		// attempted).
		if (cmd == CdlSetloc) {
			_last_pos.minute = _param[0];
			_last_pos.second = _param[1];
			_last_pos.sector = _param[2];
		} else if (cmd == CdlSetmode) {
			_last_mode = _param[0];
		}
	}

	// Request a command FIFO write.
	while (CDROM_REG(0) & 0x80)
		__asm__ volatile("");

	CDROM_REG(0) = 1;
	CDROM_REG(3) = 0x40; // Reset parameter buffer

	//while (CDROM_REG(0) & (1 << 5))
		//CDROM_REG(1);
	for (int i = 0; i < 50; i++)
		__asm__ volatile("");

	// Wait for the FIFO to become ready, then send the parameters followed by
	// the command index.
	while (CDROM_REG(0) & (1 << 7))
		__asm__ volatile("");

	CDROM_REG(0) = 0;
	for (; length > 0; length--)
		CDROM_REG(2) = *(_param++);

	CDROM_REG(0) = 0;
	CDROM_REG(1) = (uint8_t) cmd;
	return 1;
}

int CdCommand(CdlCommand cmd, const void *param, int length, uint8_t *result) {

	if (_ack_pending) {
		return 0;
	}

	_result_ptr = result;
	CdCommandF(cmd, param, length);

	// Wait for the command to be acknowledged.
	for (int i = CD_ACK_TIMEOUT; i; i--) {
		if (!_ack_pending)
			return 1;
	}

	return 0;
}

int CdMix(const CdlATV *vol) {

	CDROM_REG(0) = 2;
	CDROM_REG(2) = vol->val0;
	CDROM_REG(3) = vol->val1;

	CDROM_REG(0) = 3;
	CDROM_REG(1) = vol->val2;
	CDROM_REG(2) = vol->val3;

	CDROM_REG(3) = 0x20; // Unmute XA, apply volume changes
	return 1;
}

static void _cd_irq_handler(void) {
	CDROM_REG(0) = 1;
	CdlIntrResult irq = (CdlIntrResult)(CDROM_REG(3) & 7);

	if (irq == CdlDataReady) {
		// TODO: are the first 4 accesses really needed, or was this just
		// Sony's (dumb) way to flush the KUSEG write queue? We definitely
		// don't need to do that since we're using KSEG1.
		CDROM_REG(0) = 0;
		CDROM_REG(0);
		CDROM_REG(3) = 0;
		CDROM_REG(3);
		CDROM_REG(0) = 0;
		CDROM_REG(3) = 0x80; // Request data
	}

	CDROM_REG(0) = 1;
	CDROM_REG(3) = 0x1f; // Acknowledge all IRQs
	CDROM_REG(3) = 0x40; // Reset parameter buffer

	//while (CDROM_REG(0) & (1 << 5))
		//CDROM_REG(1);
	for (int i = 0; i < 50; i++)
		__asm__ volatile("");

	if (!irq || (irq > CdlDiskError))
		return;

	// Fetch the result from the drive if a buffer was previously set up. The
	// first byte is always read as it contains the drive status for most
	// commands.
	uint8_t first_byte = CDROM_REG(1);
	CdlCB   callback;

	if (_result_ptr) {
		_result_ptr[0] = first_byte;

		for (int i = 1; (CDROM_REG(0) & 0x20) && (i < MAX_RESULT_SIZE); i++)
			_result_ptr[i] = CDROM_REG(1);
	}

	switch (irq) {
		case CdlDataReady:
			// CdRead() can override any callback set using CdReadyCallback()
			// by setting _cd_override_callback.
			callback = _cd_override_callback;
			if (!callback)
				callback = _ready_callback;

			_update_status(first_byte);
			break;

		case CdlComplete:
			_sync_pending = 0;
			callback      = _sync_callback;

			if (_last_command <= CdlReadTOC) {
				if (_command_flags[_last_command] & C_STATUS)
					_update_status(first_byte);
			}
			break;

		case CdlAcknowledge:
			_ack_pending = 0;
			callback     = (CdlCB) 0;

			if (_last_command <= CdlReadTOC) {
				if (_command_flags[_last_command] & STATUS)
					_update_status(first_byte);
			}
			break;

		case CdlDataEnd:
			callback = _pause_callback;

			_update_status(first_byte);
			break;

		case CdlDiskError:
			_last_error = CDROM_REG(1);
			callback    = _ready_callback;

			if (_ack_pending || _sync_pending) {
				if (_sync_callback)
					_sync_callback(irq, _result_ptr);

				_ack_pending  = 0;
				_sync_pending = 0;
			}

			_update_status(first_byte);
			break;
	}

	if (callback)
		callback(irq, _result_ptr);

	_last_command = 0;
	_last_irq     = irq;
	_result_ptr   = (uint8_t *) 0;
}

void cdInterruptHandler(void) {
    if (acknowledgeInterrupt(IRQ_CDROM))
        _cd_irq_handler();
}

int CdInit(void) {
	installExceptionHandler();
	setInterruptHandler(cdInterruptHandler, ((void*)0));


	IRQ_MASK = 1 << IRQ_CDROM;
	enableInterrupts();

	BUS_CD_CFG = 0x00020943;

	SetDMAPriority(DMA_CDROM, 3);
	DMA_CHCR(DMA_CDROM) = 0x00000000; // Stop DMA

	CDROM_REG(0) = 1;
	CDROM_REG(3) = 0x1f; // Acknowledge all IRQs
	CDROM_REG(2) = 0x1f; // Enable all IRQs
	CDROM_REG(0) = 0;
	CDROM_REG(3) = 0x00; // Clear any pending request

	CdlATV mix = { 0x80, 0x00, 0x80, 0x00 };
	CdMix(&mix);

	_last_mode    = 0;
	_ack_pending  = 0;
	_sync_pending = 0;

	_cd_override_callback = (CdlCB) 0;
	_cd_media_changed     = 1;

	// Initialize the drive.
	CdCommand(CdlNop, 0, 0, 0);	
	CdCommand(CdlInit, 0, 0, 0);

	if (CdSync(0, 0) == CdlDiskError) {
		return 0;
	}

	CdCommand(CdlDemute, 0, 0, 0);

	int param = 2;
	uint8_t result[16];
	CdCommand(CdlPlay, &param, 1, result);
	return 1;
}