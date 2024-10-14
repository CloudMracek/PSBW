#pragma once

#include <stdint.h>

#define CARD_1 0
#define CARD_2 1

struct [[gnu::packed]] CARD_DIR_FRAME {
	uint32_t blockAllocationState;
	uint32_t filesize; // in bytes 8KB multiples
	uint16_t nextBlock;
	char filename[20];
	uint8_t zero = 0;
	uint8_t garbage[96];
	uint8_t checksum;
};

struct [[gnu::packed]] CARD_TITLE_FRAME {
	char magic[2];
	uint8_t iconDisplayFlag;
	uint8_t blockNumber;
	uint8_t title[64];
	uint8_t reserved1[12];
	uint8_t reserved2[16];
	uint8_t iconClut[32];
};

uint32_t card_load();
void card_save(uint32_t score);

void card_read(uint8_t card, uint16_t sectorNumber, uint8_t* data);
void card_write(uint8_t card, uint16_t sectorNumber, uint8_t* data);