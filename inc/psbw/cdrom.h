#pragma once

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

int CdInit(void);

void CdPlayCdda(int track, int loop);
void CdStopCdda();

int CdCommand(CdlCommand cmd, const void *param, int length, uint8_t *result);
