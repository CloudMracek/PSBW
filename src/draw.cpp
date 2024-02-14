#include <ps1/gpucmd.h>
#include <ps1/registers.h>
#include <ps1/system.h>
#include <stdio.h>
#include "psbw/draw.h"
#include "psbw/Sprite.h"
#include "psbw/GameObject.h"
#include "psbw/Texture.h"
#include "psbw/settings.h"

extern "C" {
	#include "psbw/vsync.h"
}
// FIX: slower but fixes uploading textures whose size is not a multiple of 16 words
#define DMA_MAX_CHUNK_SIZE 1
#define CHAIN_BUFFER_SIZE 1024


typedef struct {
	uint32_t data[CHAIN_BUFFER_SIZE];
	uint32_t *nextPacket;
} DMAChain;

DMAChain *chain;

Scene* activeScene;
void load_scene(Scene* scene) {
	activeScene = scene;
}

// Private util functions
static void gpu_gp0_wait_ready(void) {
	while (!(GPU_GP1 & GP1_STAT_CMD_READY))
		__asm__ volatile("");
}

static void dma_send_linked_list(const void *data) {

	// Wait until the GPU's DMA unit has finished sending data and is ready.
	while (DMA_CHCR(DMA_GPU) & DMA_CHCR_ENABLE)
		__asm__ volatile("");

	// Make sure the pointer is aligned to 32 bits (4 bytes). The DMA engine is
	// not capable of reading unaligned data.

	// Give DMA a pointer to the beginning of the data and tell it to send it in
	// linked list mode. The DMA unit will start parsing a chain of "packets"
	// from RAM, with each packet being made up of a 32-bit header followed by
	// zero or more 32-bit commands to be sent to the GP0 register.
	DMA_MADR(DMA_GPU) = (uint32_t) data;
	DMA_CHCR(DMA_GPU) = DMA_CHCR_WRITE | DMA_CHCR_MODE_LIST | DMA_CHCR_ENABLE;
}

uint32_t *dma_allocate_packet(DMAChain *chain, int numCommands) {
	// Grab the current pointer to the next packet then increment it to allocate
	// a new packet. We have to allocate an extra word for the packet's header,
	// which will contain the number of GP0 commands the packet is made up of as
	// well as a pointer to the next packet (or a special "terminator" value to
	// tell the DMA unit to stop).
	uint32_t *ptr      = chain->nextPacket;
	chain->nextPacket += numCommands + 1;

	// Write the header and set its pointer to point to the next packet that
	// will be allocated in the buffer.
	*ptr = gp0_tag(numCommands, chain->nextPacket);

	// Make sure we haven't yet run out of space for future packets or a linked
	// list terminator, then return a pointer to the packet's first GP0 command.
	return &ptr[1];
}

uint32_t *dma_get_chain_pointer(int numCommands) {
	return dma_allocate_packet(chain, numCommands);
}

void vram_send_data(const void *data, int x, int y, int width, int height) {
	waitForDMATransfer(DMA_GPU, 100000);

	// Calculate how many 32-bit words will be sent from the width and height of
	// the texture. If more than 16 words have to be sent, configure DMA to
	// split the transfer into 16-word chunks in order to make sure the GPU will
	// not miss any data.
	size_t length = (width * height) / 2;
	size_t chunkSize, numChunks;

	if (length < DMA_MAX_CHUNK_SIZE) {
		chunkSize = length;
		numChunks = 1;
	} else {
		chunkSize = DMA_MAX_CHUNK_SIZE;
		numChunks = length / DMA_MAX_CHUNK_SIZE;

		// Make sure the length is an exact multiple of 16 words, as otherwise
		// the last chunk would be dropped (the DMA unit does not support
		// "incomplete" chunks). Note that this will impose limitations on the
		// size of VRAM uploads.
	}

	// Put the GPU into VRAM upload mode by sending the appropriate GP0 command
	// and our coordinates.
	gpu_gp0_wait_ready();
	GPU_GP0 = gp0_vramWrite();
	GPU_GP0 = gp0_xy(x, y);
	GPU_GP0 = gp0_xy(width, height);

	// Give DMA a pointer to the beginning of the data and tell it to send it in
	// slice (chunked) mode.
	DMA_MADR(DMA_GPU) = (uint32_t) data;
	DMA_BCR (DMA_GPU) = chunkSize | (numChunks << 16);
	DMA_CHCR(DMA_GPU) = DMA_CHCR_WRITE | DMA_CHCR_MODE_SLICE | DMA_CHCR_ENABLE;
}


void tex_upload(
	Texture *info, const void *data, int x, int y, int width, int height
) {
	// Make sure the texture's size is valid. The GPU does not support textures
	// larger than 256x256 pixels.
	// Upload the texture to VRAM and wait for the process to complete.
	vram_send_data(data, x, y, width, height);
	waitForDMATransfer(DMA_GPU, 100000);
	// Update the "texpage" attribute, a 16-bit field telling the GPU several
	// details about the texture such as which 64x256 page it can be found in,
	// its color depth and how semitransparent pixels shall be blended.
	info->page = gp0_page(
		x / 64, y / 256, GP0_BLEND_SEMITRANS, GP0_COLOR_16BPP
	);

	// Calculate the texture's UV coordinates, i.e. its X/Y coordinates relative
	// to the top left corner of the texture page.
	info->u      = (uint8_t)  (x % 64);
	info->v      = (uint8_t)  (y % 256);
	info->width  = (uint16_t) width;
	info->height = (uint16_t) height;
}

void gpu_setup(GP1VideoMode mode, int width, int height) {

	DMA_DPCR |= DMA_DPCR_ENABLE << (DMA_GPU * 4);


    // Origin of framebuffer based on if PAL or NTSC
    int x = 0x760;
    int y = (mode = GP1_MODE_PAL) ? 0xa3 : 0x88;

    // We need to do some timing magic to actually achieve our desired resolution
	GP1HorizontalRes horizontalRes = GP1_HRES_320;
	GP1VerticalRes   verticalRes   = GP1_VRES_256;

    int offsetX = (width  * gp1_clockMultiplierH(horizontalRes)) / 2;
	int offsetY = (height / gp1_clockDividerV(verticalRes))      / 2;

    GPU_GP1 = gp1_resetGPU();
	GPU_GP1 = gp1_fbRangeH(x - offsetX, x + offsetX);
	GPU_GP1 = gp1_fbRangeV(y - offsetY, y + offsetY);
	GPU_GP1 = gp1_fbMode(
		horizontalRes, verticalRes, mode, false, GP1_COLOR_16BPP
	);

	GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
	GPU_GP1 = gp1_dispBlank(false);

}

static void gpu_wait_vsync(void) {
	while (!(IRQ_STAT & (1 << IRQ_VSYNC)))
		__asm__ volatile("");

	IRQ_STAT = ~(1 << IRQ_VSYNC);
}

// Engine's API

void draw_init() {
    if ((GPU_GP1 & GP1_STAT_MODE_BITMASK) == GP1_STAT_MODE_PAL) {
		gpu_setup(GP1_MODE_PAL, SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		gpu_setup(GP1_MODE_NTSC, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	GPU_GP1 = gp1_dispBlank(false);
}

bool currentBuffer = false;
DMAChain dmaChains[2];
void draw_update() {
	int frameX = currentBuffer ? SCREEN_WIDTH : 0;
	int frameY = 0;

	chain  = &dmaChains[currentBuffer];
	currentBuffer = !currentBuffer;

	uint32_t *ptr;
	GPU_GP1 = gp1_fbOffset(frameX, frameY);

	chain->nextPacket = chain->data;

	ptr = dma_allocate_packet(chain, 4);
	ptr[0] = gp0_texpage(0, true, false);
	ptr[1] = GPU_GP0 = gp0_fbOffset1(frameX, frameY);
	ptr[2] = GPU_GP0 = gp0_fbOffset2(
		frameX + SCREEN_WIDTH - 1, frameY + SCREEN_HEIGHT - 2
	);
	ptr[3] = gp0_fbOrigin(frameX, frameY);

	ptr = dma_allocate_packet(chain, 3);
	ptr[0] = gp0_rgb(64, 64, 64) | gp0_vramFill();
	ptr[1] = gp0_xy(frameX, frameY);
    ptr[2] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);

	GAMEOBJECT_ENTRY *entry = activeScene->_linked_list;
	while(entry != nullptr) {
		entry->object->execute();
		entry = entry->next;
	}

	*(chain->nextPacket) = gp0_endTag(0);
	gpu_gp0_wait_ready();
	VSync(0);
	dma_send_linked_list(chain->data);
}


