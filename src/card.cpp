#include "card.h"

#include <ps1/registers.h>
#include <stdio.h>

#include "sio0.h"
#include "vsync.h"

#include <ps1/system.h>
#include <string.h>
#include <psbw/Sio.h>

//extern const uint8_t memcardIcon[], memcardIconPalette[];

static uint8_t calculateChecksum(uint8_t a, uint8_t b, uint8_t *data, int size)
{
	uint8_t checksum = a ^ b; // Initialize checksum with XOR of the two initial values

	// Iterate over the array and XOR each element with the checksum
	for (int i = 0; i < size; i++)
	{
		checksum ^= data[i];
	}

	return checksum;
}

uint8_t calculateChecksum2(CARD_DIR_FRAME* frame) {
    const uint8_t* ptr = (const uint8_t*) frame;
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(CARD_DIR_FRAME) - 1; ++i) {
        checksum ^= ptr[i];
    }
    return checksum;
}

uint32_t card_load() {

	/*CARD_DIR_FRAME searchFrame;
	// If you have a full card RIP your savegame lol
	uint8_t firstFreeBlock = 1;
	for(int i = 1; i <= 15; i++) {
		card_read(CARD_1, i, (uint8_t *) &searchFrame);
		if(!strcmp(searchFrame.filename, "PSBWTEST")) {
			uint8_t savegame[128];
			card_read(CARD_1, i*64+2, savegame);
			uint32_t value = ((uint32_t)savegame[0] << 24) | ((uint32_t)savegame[1] << 16) | ((uint32_t)savegame[2] << 8) | savegame[3];
    		return value;
		}
	}
	return 0;*/
	uint8_t savegame[128];
	card_read(CARD_1, 66, savegame);
	uint32_t value = ((uint32_t)savegame[0] << 24) | ((uint32_t)savegame[1] << 16) | ((uint32_t)savegame[2] << 8) | savegame[3];
    return value;
}

void card_save(uint32_t score) {

	CARD_DIR_FRAME dirFrame;
	CARD_TITLE_FRAME titleFrame;

	memset(&dirFrame, 0, 128);
	memset(&titleFrame, 0, 128);

	CARD_DIR_FRAME searchFrame;
	// If you have a full card RIP your savegame lol
	uint8_t firstFreeBlock = 1;
	for(int i = 1; i <= 15; i++) {
		printf("%d\n", i);
		card_read(CARD_1, i, (uint8_t *) &searchFrame);
		if(searchFrame.blockAllocationState == 0xA0 || searchFrame.blockAllocationState == 0xA1) {
			if(i <= firstFreeBlock) {
				firstFreeBlock = i;
				printf("FOUND LOWEST BLOCK %d\n", i);
			}
		}
		else if(!strcmp(searchFrame.filename, "PSBWTEST")) {
			printf("FOUND EXISTING SAVE %d\n", i);
			firstFreeBlock = i;
			break;
		}
	}

	dirFrame.blockAllocationState = 0x51;
    dirFrame.filesize = 0x2000;    
    dirFrame.nextBlock = 0xFFFF;
    sprintf(dirFrame.filename, "PSBWTEST");

    for (int i = 0; i < 96; i++) {
        dirFrame.garbage[i] = 0x0;
    }

    dirFrame.checksum = calculateChecksum2(&dirFrame);
    card_write(CARD_1, firstFreeBlock, (uint8_t *)&dirFrame);
	card_write(CARD_1, firstFreeBlock, (uint8_t *)&dirFrame);
	card_write(CARD_1, firstFreeBlock, (uint8_t *)&dirFrame);
	printf("WRITING DIR FRAME\n");
    
    
    titleFrame.magic[0] = 'S';
    titleFrame.magic[1] = 'C';

    titleFrame.iconDisplayFlag = 0x11;
    titleFrame.blockNumber = 0x1;

    titleFrame.title[0] = 'P';
    titleFrame.title[1] = 'S';
    titleFrame.title[2] = 'B';
    titleFrame.title[3] = 'W';

    memset(titleFrame.reserved1, 0, 12);
    memset(titleFrame.reserved2, 0, 12);

    memcpy(titleFrame.iconClut, memcardIconPalette, 32);

    card_write(CARD_1, firstFreeBlock, (uint8_t *)&dirFrame);
	card_write(CARD_1, firstFreeBlock, (uint8_t *)&dirFrame);
	card_write(CARD_1, firstFreeBlock, (uint8_t *)&dirFrame);
	printf("WRITING DIR FRAME AGAIN\n");
    
    card_write(CARD_1, firstFreeBlock*64+1, (uint8_t*) &memcardIcon);
	card_write(CARD_1, firstFreeBlock*64+1, (uint8_t*) &memcardIcon);
	card_write(CARD_1, firstFreeBlock*64+1, (uint8_t*) &memcardIcon);
	printf("WRITING MEMCARD ICON\n");
    

	uint8_t savegame[128];

	savegame[0] = (score >> 24) & 0xFF;
    savegame[1] = (score >> 16) & 0xFF;
    savegame[2] = (score >> 8) & 0xFF;
    savegame[3] = score & 0xFF;

    // Filling the rest of the array with zeroes
    for (int i = 4; i < 128; i++) {
        savegame[i] = 0;
    }

	card_write(CARD_1, firstFreeBlock*64+2, savegame);
	card_write(CARD_1, firstFreeBlock*64+2, savegame);
	card_write(CARD_1, firstFreeBlock*64+2, savegame);
	printf("WRITING SAVEGAME FRAME\n");

    card_write(CARD_1, firstFreeBlock*64, (uint8_t *)&titleFrame);
    card_write(CARD_1, firstFreeBlock*64, (uint8_t *)&titleFrame);
    card_write(CARD_1, firstFreeBlock*64, (uint8_t *)&titleFrame);
}

void card_read(uint8_t card, uint16_t sectorNumber, uint8_t *data)
{
	sio0_init();
	VSync(10);
	sio0_port_select(0);
	SIO_CTRL(0) |= SIO_CTRL_DTR | SIO_CTRL_ACKNOWLEDGE;
	delayMicroseconds(60);

	SIO_DATA(0) = 0x81;
	if (!sio0_wait_acknowledge(1200))
	{
		return;
	}

	uint8_t flag = sio0_exchange_byte('R', true);
	printf("FLAG: 0x%x\n", flag);
	SIO_DATA(0);
	SIO_DATA(0);
	uint8_t card0id = sio0_exchange_byte(0x0, true);
	printf("CARD0ID: 0x%x\n", card0id);
	uint8_t card1id = sio0_exchange_byte(0x0, true);
	printf("CARD1ID: 0x%x\n", card1id);

	uint8_t msbResponse = sio0_exchange_byte((sectorNumber >> 8) & 0xFF, true); // msb
	printf("MSBRESP: 0x%x\n", msbResponse);
	uint8_t lsbResponse = sio0_exchange_byte(sectorNumber & 0xFF, true);		// lsb
	printf("LSBRESP: 0x%x\n", lsbResponse);

	uint8_t cmdAck0 = sio0_exchange_byte(0x0, true);
	printf("CMDACK0: 0x%x\n", cmdAck0);
	uint8_t cmdAck1 = sio0_exchange_byte(0x0, true);
	printf("CMDACK1: 0x%x\n", cmdAck1);

	uint8_t cfrmdAddrMSB = sio0_exchange_byte(0x0, true);
	printf("MSB: 0x%x\n", cfrmdAddrMSB);
	uint8_t cfrmdAddrLSB = sio0_exchange_byte(0x0, true);
	printf("LSB: 0x%x\n", cfrmdAddrLSB);

	for (int i = 0; i < 128; i++)
	{
		data[i] = sio0_exchange_byte(0x0, true);
	}

	uint8_t checksum = sio0_exchange_byte(0x0, true);
	printf("SUM: 0x%x\n", checksum);
	uint8_t endByte = sio0_exchange_byte(0x0, false);
	printf("END: 0x%x\n", endByte);
	VSync(10);
	sio0_init();
}

void card_write(uint8_t card, uint16_t sectorNumber, uint8_t *data)
{
	sio0_init();
	VSync(10);
	sio0_port_select(0);
	SIO_CTRL(0) |= SIO_CTRL_DTR | SIO_CTRL_ACKNOWLEDGE;
	delayMicroseconds(60);

	SIO_DATA(0) = 0x81;
	if (!sio0_wait_acknowledge(1200))
	{
		return;
	}
	

	sio0_exchange_byte('W', true);

	SIO_DATA(0);
	SIO_DATA(0);

	uint8_t card0id = sio0_exchange_byte(0x0, true);
	printf("CARD0ID: 0x%x\n", card0id);
	uint8_t card1id = sio0_exchange_byte(0x0, true);
	printf("CARD1ID: 0x%x\n", card1id);
	uint8_t msb = (sectorNumber >> 8) & 0xFF;

	uint8_t lsb = sectorNumber & 0xFF;

	sio0_exchange_byte(msb, true); // msb
	sio0_exchange_byte(lsb, true); // lsb

	for (int i = 0; i < 128; i++)
	{
		sio0_exchange_byte(data[i], true);
	}

	sio0_exchange_byte(calculateChecksum(msb, lsb, data, 128), true);

	uint8_t cmdAck0 = sio0_exchange_byte(0x0, true);
	printf("CMDACK0: 0x%x\n", cmdAck0);
	uint8_t cmdAck1 = sio0_exchange_byte(0x0, true);
	printf("CARD0ID: 0x%x\n", cmdAck1);
	uint8_t endByte = sio0_exchange_byte(0x0, false);
	printf("END: 0x%x\n", endByte);
	VSync(10);
	sio0_init();
}