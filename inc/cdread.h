#pragma once
#include "cdrom.h"

#ifdef __cplusplus
extern "C" {
#endif

int CdGetSector(void *madr, int size);

int CdReadRetry(int sectors, uint32_t *buf, int mode, int attempts);

int CdRead(int sectors, uint32_t *buf, int mode);

void CdReadBreak(void);

int CdReadSync(int mode, uint8_t *result);

CdlCB CdReadCallback(CdlCB func);

#ifdef __cplusplus
}
#endif
