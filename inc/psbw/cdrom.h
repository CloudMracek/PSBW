#pragma once
#include "ps1/registers.h"

#define btoi(b) (((b) / 16 * 10) + ((b) % 16))
#define itob(i) (((i) / 10 * 16) | ((i) % 10))

typedef enum {
	CdlIsoOkay		= 0,	// File system parser okay.
	CdlIsoSeekError	= 1,	// Logical seek error occurred. May occur when attempting to query the filesystem on an audio-only CD.
	CdlIsoReadError	= 2,	// Read error occurred while reading the CD-ROM file system descriptor.
	CdlIsoInvalidFs	= 3,	// Disc does not contain a standard ISO9660 file system.
	CdlIsoLidOpen	= 4		// Lid is open when attempting to parse the CD-ROM file system.
} CdlIsoError;

typedef enum
{
	CdlNoIntr = 0,		// No pending interrupt
	CdlDataReady = 1,	// INT1 (new sector or CD-DA report packet available)
	CdlComplete = 2,	// INT2 ("complete" response received, blocking command has finished)
	CdlAcknowledge = 3, // INT3 ("acknowledge" response received, non-blocking command has finished or blocking command has started)
	CdlDataEnd = 4,		// INT4 (end of track or end of disc reached)
	CdlDiskError = 5	// INT5 (command error, read error or lid opened)
} CdlIntrResult;

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

typedef struct
{
	uint8_t minute; // Minutes (BCD)
	uint8_t second; // Seconds (BCD)
	uint8_t sector; // Sector or frame (BCD)
	uint8_t track;	// Track number
} CdlLOC;

typedef struct {
	CdlLOC	pos;		// CD-ROM position coordinates of file
	int		size;		// Size of file in bytes
	char	name[16];	// File name
} CdlFILE;

typedef enum {
	CdlModeDA		= 1 << 0,	// Enable CD-DA playback.
	CdlModeAP		= 1 << 1,	// Automatically pause CD-DA playback and invoke any callback set by CdAutoPauseCallback() when the end of a track is reached.
	CdlModeRept		= 1 << 2,	// Automatically send report packets containing the current drive status and position during CD-DA playback.
	CdlModeSF		= 1 << 3,	// Enable XA sector filter. When set, only XA-ADPCM sectors that match the file and channel numbers set using CdlSetloc will be played.
	//CdlModeSize0	= 1 << 4,
	//CdlModeSize1	= 1 << 5,
	CdlModeIgnore	= 1 << 4,	// Ignore any previously set sector size (CdlModeSize flag) and location set using CdlSetloc.
	CdlModeSize		= 1 << 5,	// Return 2340 bytes instead of 2048 for each data sector read. The data returned is a full 2352-byte CD sector minus the sync bytes at the beginning.
	CdlModeRT		= 1 << 6,	// Enable XA-ADPCM playback. When not set, XA-ADPCM sectors will be read as normal data.
	CdlModeSpeed	= 1 << 7	// Read sectors at 2x speed instead of the default 1x. Should be cleared for CD-DA playback.
} CdlModeFlag;

int CdInit(void);

int CdControlF(CdlCommand cmd, const void *param);
int CdControl(CdlCommand cmd, const void *param, uint8_t *result);
int CdControlB(CdlCommand cmd, const void *param, uint8_t *result);

int CdStatus(void);

const CdlLOC *CdLastPos(void);

CdlLOC *CdIntToPos(int i, CdlLOC *p);

int CdPosToInt(const CdlLOC *p);

void CdPlayCdda(int track, int loop);
void CdStopCdda();

CdlIntrResult CdSync(int mode, uint8_t *result);

int CdCommandF(CdlCommand cmd, const void *param, int length);

int CdCommand(CdlCommand cmd, const void *param, int length, uint8_t *result);
