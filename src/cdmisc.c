/*
 * PSn00bSDK CD-ROM library (misc. functions)
 * (C) 2020-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#include "cdrom.h"

#include <stdint.h>
#include <string.h>

#include <ps1/registers.h>

#define DATA_SYNC_TIMEOUT 0x100000

/* Unlock command strings */

static const char *_unlock_strings[] = {
	"",
	"Licensed by",
	"Sony",
	"Computer",
	"Entertainment",
	"",
	""};

static const char *const _unlock_regions[] = {
	"of America", // CdlRegionSCEA
	"(Europe)",	  // CdlRegionSCEE
	"World wide"  // CdlRegionSCEW
};

uint8_t cdda_loop = 0, cdda_current_track = -1;

/* Sector DMA transfer functions */

int CdGetSector(void *madr, int size)
{

	// while (!(CDROM_REG(0) & (1 << 6)))
	//__asm__ volatile("");

	DMA_MADR(DMA_CDROM) = (uint32_t)madr;
	DMA_BCR(DMA_CDROM) = size | (1 << 16);
	DMA_CHCR(DMA_CDROM) = 0x11000000;

	while (DMA_CHCR(DMA_CDROM) & (1 << 24))
		__asm__ volatile("");

	return 1;
}

int CdGetSector2(void *madr, int size)
{

	// while (!(CDROM_REG(0) & (1 << 6)))
	//__asm__ volatile("");

	DMA_MADR(DMA_CDROM) = (uint32_t)madr;
	DMA_BCR(DMA_CDROM) = size | (1 << 16);
	DMA_CHCR(DMA_CDROM) = 0x11400100; // Transfer 1 word every 16 CPU cycles

	return 1;
}

int CdDataSync(int mode)
{
	if (mode)
		return (DMA_CHCR(DMA_CDROM) >> 24) & 1;

	for (int i = DATA_SYNC_TIMEOUT; i; i--)
	{
		if (!(DMA_CHCR(DMA_CDROM) & (1 << 24)))
			return 0;
	}

	printf("CdDataSync() timeout, CHCR=0x%08x\n", DMA_CHCR(DMA_CDROM));
	return -1;
}

/* LBA/MSF conversion */

CdlLOC *CdIntToPos(int i, CdlLOC *p)
{
	i += 150;

	p->minute = itob(i / (75 * 60));
	p->second = itob((i / 75) % 60);
	p->sector = itob(i % 75);
	return p;
}

int CdPosToInt(const CdlLOC *p)
{
	return (
			   (btoi(p->minute) * (75 * 60)) +
			   (btoi(p->second) * 75) +
			   btoi(p->sector)) -
		   150;
}

/* Drive unlocking API */

CdlRegionCode CdGetRegion(void)
{
	uint8_t param;
	uint8_t result[16];

	// Firmware version C0 does not support test command 0x22 to retrieve the
	// region, but it was only used in the SCPH-1000 Japanese model. Version D1
	// (and possibly others?) is used in debug consoles.
	// https://psx-spx.consoledev.net/cdromdrive/#19h20h-int3yymmddver
	// https://psx-spx.consoledev.net/cdromdrive/#19h22h-int3for-europe
	param = 0x20;
	memset(result, 0, 4);

	if (!CdCommand(CdlTest, &param, 1, result))
	{
		printf("failed to probe drive firmware version\n");
		return CdlRegionUnknown;
	}

	printf("drive firmware version: 0x%02x\n", result[3]);
	if (result[3] == 0xc0)
		return CdlRegionSCEI;
	if (result[3] >= 0xd0)
		return CdlRegionDebug;

	param = 0x22;
	memset(result, 0, 16);

	if (!CdCommand(CdlTest, &param, 1, result))
	{
		printf("failed to probe drive region\n");
		return CdlRegionUnknown;
	}

	printf("drive region: %s\n", result);
	if (!strcmp(result, "for Japan"))
		return CdlRegionSCEI;
	if (!strcmp(result, "for U/C"))
		return CdlRegionSCEA;
	if (!strcmp(result, "for Europe"))
		return CdlRegionSCEE;
	if (!strcmp(result, "for NETNA") || !strcmp(result, "for NETEU"))
		return CdlRegionSCEW;
	if (!strcmp(result, "for US/AEP"))
		return CdlRegionDebug;

	return CdlRegionUnknown;
}

int CdUnlock(CdlRegionCode region)
{
	if (region <= CdlRegionSCEI)
		return 0;
	if (region >= CdlRegionDebug)
		return 1;

	// This is by far the most efficient way to do it.
	_unlock_strings[5] = _unlock_regions[region - CdlRegionSCEA];

	for (int i = 0; i < 7; i++)
	{
		uint8_t result[4];

		if (!CdCommand(
				0x50 + i,
				_unlock_strings[i],
				strlen(_unlock_strings[i]),
				result))
			return 0;

		if (!(result[0] & CdlStatError) || (result[1] != 0x40))
		{
			printf("unlock failed, status=0x%02x, code=0x%02x\n", result[0], result[1]);
			return 0;
		}
	}

	printf("unlock successful\n");
	return CdCommand(CdlNop, 0, 0, 0);
}

/* Misc. functions */

int CdGetToc(CdlLOC *toc)
{

	uint8_t result[4];

	if (!CdCommand(CdlGetTN, 0, 0, result))
		return 0;
	if (CdSync(1, 0) != CdlComplete)
		return 0;

	int first = btoi(result[1]);
	int tracks = btoi(result[2]) + 1 - first;
	// assert(first == 1);

	for (int i = 0; i < tracks; i++)
	{
		uint8_t track = itob(first + i);

		if (!CdCommand(CdlGetTD, &track, 1, result))
			return 0;
		if (CdSync(1, 0) != CdlComplete)
			return 0;

		toc[i].minute = result[1];
		toc[i].second = result[2];
		toc[i].sector = 0;
		toc[i].track = track;
	}

	return tracks;
}

int CdMix(const CdlATV *vol)
{

	CDROM_REG(0) = 2;
	CDROM_REG(2) = vol->val0;
	CDROM_REG(3) = vol->val1;

	CDROM_REG(0) = 3;
	CDROM_REG(1) = vol->val2;
	CDROM_REG(2) = vol->val3;

	CDROM_REG(3) = 0x20; // Unmute XA, apply volume changes
	return 1;
}

void _cdda_end_callback()
{
	if (cdda_loop)
	{
		CdPlayCdda(cdda_current_track, 1);
	}
	return;
}

void CdReplayCdda()
{
	if (cdda_current_track != -1)
	{
		CdPlayCdda(cdda_current_track, cdda_loop);
	}
}

int first = 1;
void CdPlayCdda(int track, int loop)
{
	if(cdda_current_track != track || cdda_loop == 0) {
		first = 1;
	}

	if(first) {
		uint8_t _mode = CdlModeAP;
		CdCommand(CdlSetmode, &_mode, 1, 0);
		CdAutoPauseCallback(_cdda_end_callback);
		first = 0;
	}
	
	cdda_loop = loop;
	int param = track;
	uint8_t result[16];
	CdCommand(CdlPlay, &param, 1, result);
	cdda_current_track = track;
}

void CdStopCdda()
{
	cdda_loop = 0;
	cdda_current_track = -1;
	CdCommand(CdlPause, 0, 0, 0);
}
