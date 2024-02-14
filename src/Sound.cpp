#include "psbw/Sound.h"
#include "ps1/system.h"
#include "ps1/registers.h"
#include "vendor/printf.h"

extern "C"
{
#include "psbw/cdrom.h"
#include "psbw/Sound.h"
}

#define _min(x, y) (((x) < (y)) ? (x) : (y))
#define getSPUAddr(addr) ((uint16_t)(((addr) + 7) / 8))

#define SpuSetKey(enable, voice_bit)                              \
	((enable) ? (                                                 \
					SPU_FLAG_ON1 = (uint16_t)(voice_bit),         \
					SPU_FLAG_ON2 = (uint16_t)((voice_bit) >> 16)) \
			  : (                                                 \
					SPU_FLAG_OFF1 = (uint16_t)(voice_bit),        \
					SPU_FLAG_OFF2 = (uint16_t)((voice_bit) >> 16)))

#define getSPUSampleRate(rate) ((uint16_t)(((rate) * (1 << 12)) / 44100))

typedef struct
{
	uint32_t magic; // 0x70474156 ("VAGp") for mono files
	uint32_t version;
	uint32_t interleave;  // Unused in mono files
	uint32_t size;		  // Big-endian, in bytes
	uint32_t sample_rate; // Big-endian, in Hertz
	uint16_t _reserved[5];
	uint16_t channels; // Unused in mono files
	char name[16];
} VAG_Header;

// Private API

static const uint32_t _dummy_block[4] = {
	0x00000500, 0x00000000, 0x00000000, 0x00000000};

static int next_sample_addr = 0x1000 + sizeof(_dummy_block);

static void spu_wait_status(uint16_t mask, uint16_t value)
{
	for (int i = 0x100000; i; i--)
	{
		if ((SPU_STAT & mask) == value)
			return;
	}
}

#define _DMA_CHUNK_SIZE 4 // 16 bytes

static int spu_dma_transfer(uint32_t ramOffset, const void *data, size_t length, bool wait)
{
	length /= 4;

	// assert(!(length % _DMA_CHUNK_SIZE));
	length = (length + _DMA_CHUNK_SIZE - 1) / _DMA_CHUNK_SIZE;

	if (!waitForDMATransfer(DMA_SPU, 100000))
		return 0;

	uint16_t ctrlReg = SPU_CTRL & ~SPU_CTRL_XFER_BITMASK;

	SPU_CTRL = ctrlReg;
	spu_wait_status(SPU_CTRL_XFER_BITMASK, 0);

	SPU_DMA_CTRL = 4;
	SPU_ADDR = ramOffset / 8;
	SPU_CTRL = ctrlReg | SPU_CTRL_XFER_DMA_WRITE;
	spu_wait_status(SPU_CTRL_XFER_BITMASK, SPU_CTRL_XFER_DMA_WRITE);

	DMA_MADR(DMA_SPU) = reinterpret_cast<uint32_t>(data);
	DMA_BCR(DMA_SPU) = _DMA_CHUNK_SIZE | (length << 16);
	DMA_CHCR(DMA_SPU) = DMA_CHCR_WRITE | DMA_CHCR_MODE_SLICE | DMA_CHCR_ENABLE;

	if (wait)
		waitForDMATransfer(DMA_SPU, 100000);

	return length * _DMA_CHUNK_SIZE * 4;
}

void spu_init()
{

	DMA_DPCR |= DMA_DPCR_ENABLE << (DMA_SPU * 4);
	BIU_DEV4_CTRL = 0x200931e1;

	SPU_CTRL = 0x0000;
	spu_wait_status(0x001f, 0x0000);

	SPU_MASTER_VOL_L = 0;
	SPU_MASTER_VOL_R = 0;
	SPU_REVERB_VOL_L = 0;
	SPU_REVERB_VOL_R = 0;
	SPU_FLAG_OFF1 = 0xffff;
	SPU_FLAG_OFF2 = 0x00ff;
	SPU_FLAG_FM1 = 0;
	SPU_FLAG_FM2 = 0;
	SPU_FLAG_NOISE1 = 0;
	SPU_FLAG_NOISE2 = 0;
	SPU_FLAG_REVERB2 = 0;
	SPU_FLAG_REVERB1 = 0;
	SPU_REVERB_ADDR = 0xfffe;
	SPU_CDDA_VOL_L = 0;
	SPU_CDDA_VOL_R = 0;
	SPU_EXT_VOL_L = 0;
	SPU_EXT_VOL_R = 0;

	SPU_DMA_CTRL = 0x0004; // Reset transfer mode
	SPU_CTRL = 0xc001;	   // Enable SPU, DAC, CD audio, disable DMA request
	spu_wait_status(0x003f, 0x0001);

	// Upload a dummy looping ADPCM block to the first 16 bytes of SPU RAM.
	// This may be freely used or overwritten.
	spu_dma_transfer(0x1000, _dummy_block, sizeof(_dummy_block), true);

	for (int i = 0; i < 24; i++)
	{
		SPU_CH_VOL_L(i) = 0;
		SPU_CH_VOL_R(i) = 0;
		SPU_CH_FREQ(i) = getSPUSampleRate(44100);
		SPU_CH_ADDR(i) = getSPUAddr(0x1000);
	}

	SPU_FLAG_ON1 = 0xffff;
	SPU_FLAG_ON2 = 0x00ff;
	SPU_MASTER_VOL_L = 0x3fff;
	SPU_MASTER_VOL_R = 0x3fff;
	SPU_CDDA_VOL_L = 0x7fff;
	SPU_CDDA_VOL_R = 0x7fff;
}

int spu_is_transfer_completed(int mode)
{
	if (!mode)
		return ((SPU_STAT >> 10) & 1) ^ 1;

	spu_wait_status(0x0400, 0x0000);
	return 1;
}

int get_free_channel(void)
{
#if 0
	// Pick a channel that has looped/stopped at least once
	// Broken in pcsx-redux, will also break if looping sounds are used
	uint32_t flags = SPU_FLAG_STATUS1 | (SPU_FLAG_STATUS2 << 16);

	for (int ch = 0; flags; ch++, flags >>= 1) {
		if (flags & 1)
			return ch;
	}
#else
	// Pick a channel whose volume envelope is currently idle
	for (int ch = 0; ch < 24; ch++)
	{
		if (!SPU_CH_ADSR_VOL(ch))
			return ch;
	}
#endif

	return -1;
}

// Public API

Sound::Sound(const void *data)
{
	spu_upload_sample(data);
}


Sound::Sound()
{
}

void spu_upload(const void* data, size_t size) {
	spu_dma_transfer(0x1000, data, size, true);
}

void Sound::spu_upload_sample(const void *data)
{
	// Round the size up to the nearest multiple of 64, as SPU DMA transfers
	// are done in 64-byte blocks.

	VAG_Header *vag = (VAG_Header *)data;

	sampleRate = __builtin_bswap32(vag->sample_rate);

	const uint8_t *rawData = (const uint8_t *)data;
	rawData += sizeof(VAG_Header);

	int addr = next_sample_addr;
	int size = __builtin_bswap32(vag->size);
	// int _size = (__builtin_bswap32(vag->size) + 63) & ~63;

	spu_dma_transfer(addr, rawData, size, true);
	spu_is_transfer_completed(1);

	next_sample_addr = addr + size;
	soundAddr = addr;
}

void Sound::play()
{
	int ch = get_free_channel();

	// Make sure the channel is stopped.
	SpuSetKey(0, 1 << ch);

	// Set the channel's sample rate and start address. Note that the SPU
	// expects the sample rate to be in 4.12 fixed point format (with
	// 1.0 = 44100 Hz) and the address in 8-byte units; psxspu.h provides the
	// getSPUSampleRate() and getSPUAddr() macros to convert values to these
	// units.
	SPU_CH_FREQ(ch) = sampleRate;
	SPU_CH_ADDR(ch) = (soundAddr)+0x0200;

	// Set the channel's volume and ADSR parameters (0x80ff and 0x1fee are
	// dummy values that disable the ADSR envelope entirely).
	SPU_CH_VOL_L(ch) = 0x3fff;
	SPU_CH_VOL_R(ch) = 0x3fff;
	SPU_CH_ADSR1(ch) = 0x00ff;
	SPU_CH_ADSR2(ch) = 0x0000;

	// Start the channel.
	SpuSetKey(1, 1 << ch);
}

void sound_play_cdda(int track, int loop)
{
	CdPlayCdda(track, loop);
}

void sound_stop_cdda()
{
	CdStopCdda();
}
