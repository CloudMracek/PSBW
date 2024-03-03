#include "draw.h"

#include <stdio.h>

#include <ps1/gpucmd.h>
#include <ps1/registers.h>
#include <ps1/system.h>

#include "vsync.h"
#include "gte.h"

#include "psbw/Sprite.h"
#include "psbw/GameObject.h"
#include "psbw/Texture.h"
#include "psbw/Font.h"

// FIX: slower but fixes uploading textures whose size is not a multiple of 16 words
#define DMA_MAX_CHUNK_SIZE 1
#define CHAIN_BUFFER_SIZE 4096
#define ORDERING_TABLE_SIZE 32

typedef struct
{
	uint32_t data[CHAIN_BUFFER_SIZE];
	uint32_t orderingTable[ORDERING_TABLE_SIZE];
	uint32_t *nextPacket;
} DMAChain;

DMAChain *chain;
uint8_t _graphicsMode;

#define FONT_WIDTH 96
#define FONT_HEIGHT 56
#define FONT_COLOR_DEPTH GP0_COLOR_4BPP
extern const uint8_t debugFont[], debugFontPalette[];
Texture *debugFontTexture;
Font *font;

Scene *activeScene;

void clearOrderingTable(uint32_t *table, int numEntries) {
	// Set up the OTC DMA channel to transfer a new empty ordering table to RAM.
	// The table is always reversed and generated "backwards" (the last item in
	// the table is the first one that will be written), so we must give DMA a
	// pointer to the end of the table rather than its beginning.
	DMA_MADR(DMA_OTC) = (uint32_t) &table[numEntries - 1];
	DMA_BCR (DMA_OTC) = numEntries;
	DMA_CHCR(DMA_OTC) = 0
		| DMA_CHCR_READ | DMA_CHCR_REVERSE | DMA_CHCR_MODE_BURST
		| DMA_CHCR_ENABLE | DMA_CHCR_TRIGGER;

	// Wait for DMA to finish generating the table.
	while (DMA_CHCR(DMA_OTC) & DMA_CHCR_ENABLE)
		__asm__ volatile("");
}

void uploadIndexedTexture(
	Texture *info, const void *image, const void *palette, int x, int y,
	int paletteX, int paletteY, int width, int height, GP0ColorDepth colorDepth)
{
	int numColors = (colorDepth == GP0_COLOR_8BPP) ? 256 : 16;
	int widthDivider = (colorDepth == GP0_COLOR_8BPP) ? 2 : 4;

	vram_send_data(image, x, y, width / widthDivider, height);
	waitForDMATransfer(DMA_GPU, 100000);
	vram_send_data(palette, paletteX, paletteY, numColors, 1);
	waitForDMATransfer(DMA_GPU, 100000);

	info->page = gp0_page(
		x / 64, y / 256, GP0_BLEND_SEMITRANS, colorDepth);
	info->clut = gp0_clut(paletteX / 16, paletteY);
	info->u = (uint8_t)((x % 64) * widthDivider);
	info->v = (uint8_t)(y % 256);
	info->width = (uint16_t)width;
	info->height = (uint16_t)height;
}

void upload_debug_font() {
	// Upload the debugFont to the vram
	if(debugFontTexture == nullptr) {
		debugFontTexture = new Texture();
	}
	uploadIndexedTexture(
		debugFontTexture, debugFont, debugFontPalette, SCREEN_WIDTH * 2, 0, SCREEN_WIDTH * 2,
		FONT_HEIGHT, FONT_WIDTH, FONT_HEIGHT, FONT_COLOR_DEPTH);
	if(font == nullptr) {
	font = new Font(debugFontTexture);
	}
	else {
		delete font;
		font = new Font(debugFontTexture);
	}
}

void load_scene(Scene *scene)
{
	upload_debug_font();
	draw_update(false);
	draw_update(false);
	if (activeScene != nullptr)
	{
		delete activeScene;
	}

	if(scene->type == SCENE_3D) {
		gte_setup_3d(SCREEN_WIDTH, SCREEN_HEIGHT, ORDERING_TABLE_SIZE);
	}

	scene->loadData();
	activeScene = scene;
	update_random_seed();
	activeScene->sceneSetup();
}

// Private util functions
static void gpu_gp0_wait_ready(void)
{
	while (!(GPU_GP1 & GP1_STAT_CMD_READY))
		__asm__ volatile("");
}

static void dma_send_linked_list(const void *data)
{

	// Wait until the GPU's DMA unit has finished sending data and is ready.
	while (DMA_CHCR(DMA_GPU) & DMA_CHCR_ENABLE)
		__asm__ volatile("");

	// Make sure the pointer is aligned to 32 bits (4 bytes). The DMA engine is
	// not capable of reading unaligned data.

	// Give DMA a pointer to the beginning of the data and tell it to send it in
	// linked list mode. The DMA unit will start parsing a chain of "packets"
	// from RAM, with each packet being made up of a 32-bit header followed by
	// zero or more 32-bit commands to be sent to the GP0 register.
	DMA_MADR(DMA_GPU) = (uint32_t)data;
	DMA_CHCR(DMA_GPU) = DMA_CHCR_WRITE | DMA_CHCR_MODE_LIST | DMA_CHCR_ENABLE;
}

uint32_t *dma_allocate_packet(DMAChain *chain, int numCommands, int zIndex)
{
	// Grab the current pointer to the next packet then increment it to allocate
	// a new packet. We have to allocate an extra word for the packet's header,
	// which will contain the number of GP0 commands the packet is made up of as
	// well as a pointer to the next packet (or a special "terminator" value to
	// tell the DMA unit to stop).
	uint32_t *ptr = chain->nextPacket;
	chain->nextPacket += numCommands + 1;

	*ptr = gp0_tag(numCommands, (void *) chain->orderingTable[zIndex]);
	chain->orderingTable[zIndex] = gp0_tag(0, ptr);

	return &ptr[1];
}

uint32_t *dma_get_chain_pointer(int numCommands, int zIndex)
{
	return dma_allocate_packet(chain, numCommands, zIndex);
}

void vram_send_data(const void *data, int x, int y, int width, int height)
{
	waitForDMATransfer(DMA_GPU, 100000);

	// Calculate how many 32-bit words will be sent from the width and height of
	// the texture. If more than 16 words have to be sent, configure DMA to
	// split the transfer into 16-word chunks in order to make sure the GPU will
	// not miss any data.
	size_t length = (width * height) / 2;
	size_t chunkSize, numChunks;

	if (length < DMA_MAX_CHUNK_SIZE)
	{
		chunkSize = length;
		numChunks = 1;
	}
	else
	{
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
	DMA_MADR(DMA_GPU) = (uint32_t)data;
	DMA_BCR(DMA_GPU) = chunkSize | (numChunks << 16);
	DMA_CHCR(DMA_GPU) = DMA_CHCR_WRITE | DMA_CHCR_MODE_SLICE | DMA_CHCR_ENABLE;
}

void gpu_setup(GP1VideoMode mode, int width, int height)
{

	DMA_DPCR |= DMA_DPCR_ENABLE << (DMA_GPU * 4);
	DMA_DPCR |= DMA_DPCR_ENABLE << (DMA_OTC * 4);

	// Origin of framebuffer based on if PAL or NTSC
	int x = 0x760;
	int y = (mode = GP1_MODE_PAL) ? 0xa3 : 0x88;

	// We need to do some timing magic to actually achieve our desired resolution
	GP1HorizontalRes horizontalRes = GP1_HRES_320;
	GP1VerticalRes verticalRes = GP1_VRES_256;

	int offsetX = (width * gp1_clockMultiplierH(horizontalRes)) / 2;
	int offsetY = (height / gp1_clockDividerV(verticalRes)) / 2;

	GPU_GP1 = gp1_resetGPU();
	GPU_GP1 = gp1_fbRangeH(x - offsetX, x + offsetX);
	GPU_GP1 = gp1_fbRangeV(y - offsetY, y + offsetY);
	GPU_GP1 = gp1_fbMode(
		horizontalRes, verticalRes, mode, false, GP1_COLOR_16BPP);

	GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
	GPU_GP1 = gp1_dispBlank(false);
}

static void gpu_wait_vsync(void)
{
	while (!(IRQ_STAT & (1 << IRQ_VSYNC)))
		__asm__ volatile("");

	IRQ_STAT = ~(1 << IRQ_VSYNC);
}

// Engine's API

void draw_init()
{
	if ((GPU_GP1 & GP1_STAT_MODE_BITMASK) == GP1_STAT_MODE_PAL)
	{
		gpu_setup(GP1_MODE_PAL, SCREEN_WIDTH, SCREEN_HEIGHT);
		_graphicsMode = GRAPHICS_MODE_PAL;
	}
	else
	{
		gpu_setup(GP1_MODE_NTSC, SCREEN_WIDTH, SCREEN_HEIGHT);
		_graphicsMode = GRAPHICS_MODE_NTSC;
	}
	GPU_GP1 = gp1_dispBlank(false);
}

bool currentBuffer = false;
DMAChain dmaChains[2];
void draw_update(bool doGameTick)
{
	if (doGameTick)
		activeScene->sceneLoop();

	int frameX = currentBuffer ? SCREEN_WIDTH : 0;
	int frameY = 0;

	chain = &dmaChains[currentBuffer];
	currentBuffer = !currentBuffer;

	uint32_t *ptr;
	GPU_GP1 = gp1_fbOffset(frameX, frameY);

	clearOrderingTable(chain->orderingTable, ORDERING_TABLE_SIZE);
	chain->nextPacket = chain->data;

	if (!doGameTick)
	{
		ptr = dma_allocate_packet(chain, 3, ORDERING_TABLE_SIZE-1);
		ptr[0] = gp0_rgb(0, 0, 0) | gp0_vramFill();
		ptr[1] = gp0_xy(frameX, frameY);
		ptr[2] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	else if (activeScene->backgroundImage == nullptr)
	{
		ptr = dma_allocate_packet(chain, 3, ORDERING_TABLE_SIZE-1);
		ptr[0] = gp0_rgb(64, 64, 64) | gp0_vramFill();
		ptr[1] = gp0_xy(frameX, frameY);
		ptr[2] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	else
	{
		ptr = dma_allocate_packet(chain, 4, ORDERING_TABLE_SIZE-1);
		ptr[0] = gp0_vramBlit();
		ptr[1] = gp0_xy(activeScene->backgroundImage->x, activeScene->backgroundImage->y);
		ptr[2] = gp0_xy(frameX, frameY);
		ptr[3] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	ptr = dma_allocate_packet(chain, 4, ORDERING_TABLE_SIZE-1);
	ptr[0] = gp0_texpage(0, true, false);
	ptr[1] = GPU_GP0 = gp0_fbOffset1(frameX, frameY);
	ptr[2] = GPU_GP0 = gp0_fbOffset2(
		frameX + SCREEN_WIDTH - 1, frameY + SCREEN_HEIGHT - 1);

	ptr[3] = gp0_fbOrigin(frameX, frameY);

	if (doGameTick)
	{
		GAMEOBJECT_ENTRY *entry = &activeScene->_linked_list;
		while (entry != nullptr && entry->object != nullptr)
		{
			entry->object->execute();
			entry = entry->next;
		}
	}
	else {
		font->printString(135,110, "LOADING...", 0);
	}

	*(chain->nextPacket) = gp0_endTag(0);
	gpu_gp0_wait_ready();
	VSync(0);
	dma_send_linked_list(&(chain->orderingTable)[ORDERING_TABLE_SIZE - 1]);
	waitForDMATransfer(DMA_GPU, 100000);
}

uint8_t draw_get_graphics_mode()
{
	return _graphicsMode;
}

int getOtSize() {
	return ORDERING_TABLE_SIZE;
}
