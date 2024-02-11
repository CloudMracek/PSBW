#include "psbw/filesystem.h"
#include "psbw/draw.h"
#include "ps1/system.h"
#include "string.h"
#include <stdlib.h>

#define CD_READ_TIMEOUT		180
#define CD_READ_COOLDOWN	60
#define CD_READ_ATTEMPTS	3

#define IS_PATH_SEP(ch)		(((ch) == '/') || ((ch) == '\\'))
#define DEFAULT_PATH_SEP	'\\'

extern volatile int _cd_media_changed;

static int			_cd_iso_last_dir_lba;
static uint8_t		_cd_iso_descriptor_buff[2048];
static uint8_t		*_cd_iso_pathtable_buff=NULL;
static uint8_t		*_cd_iso_directory_buff=NULL;
static int			_cd_iso_directory_len;
static CdlIsoError	_cd_iso_error=CdlIsoOkay;

static CdlCB _read_callback = (CdlCB) 0;

static int     _total_sectors, _sector_size;
static uint8_t _read_result[4];

static volatile uint32_t *_read_addr;
static volatile int      _read_timeout, _pending_attempts, _pending_sectors;

extern CdlCB _cd_override_callback;

int CdGetSector(void *madr, int size) {
	//while (!(CD_REG(0) & (1 << 6)))
		//__asm__ volatile("");

	DMA_MADR(DMA_CDROM) = (uint32_t) madr;
	DMA_BCR(DMA_CDROM)  = size | (1 << 16);
	DMA_CHCR(DMA_CDROM) = 0x11000000;

	while (DMA_CHCR(DMA_CDROM) & (1 << 24))
		__asm__ volatile("");

	return 1;
}

static void _sector_callback(CdlIntrResult irq, uint8_t *result) {
	if (irq == CdlDataReady) {
		CdGetSector((void *) _read_addr, _sector_size);
		_read_addr += _sector_size;

		if (--_pending_sectors > 0) {
			_read_timeout = VSync(-1) + CD_READ_TIMEOUT;
			return;
		}
	}

	// Stop reading if an error occurred or if no more sectors need to be read.
	CdCommandF(CdlPause, 0, 0);

	_cd_override_callback = (CdlCB) 0;
	if ((!_pending_sectors || !_pending_attempts) && _read_callback)
		_read_callback(irq, result);

	_read_timeout = VSync(-1) + CD_READ_COOLDOWN;
}

static int _poll_retry(void) {
	if (!_pending_attempts) {

		_pending_sectors = 0;
		return -1;
	}
	_pending_attempts--;

	CdlLOC pos;
	CdIntToPos(
		CdPosToInt(CdLastPos()) + _total_sectors - _pending_sectors,
		&pos
	);

	_read_timeout  = VSync(-1) + CD_READ_TIMEOUT;
	_total_sectors = _pending_sectors;

	disableInterrupts();
	_cd_override_callback = &_sector_callback;
	enableInterrupts();

	if (CdCommand(CdlSetloc, (uint8_t *) &pos, 3, _read_result))
		CdCommand(CdlReadN, 0, 0, _read_result);

	return _pending_sectors;
}


int CdReadSync(int mode, uint8_t *result) {
	if (mode) {
		if (_pending_sectors < 0)
			return -2;
		if (!_pending_sectors)
			return 0;

		if (VSync(-1) > _read_timeout)
			return _poll_retry();
		if (CdSync(1, 0) == CdlDiskError)
			return -1;

		return _pending_sectors;
	}

	while (_pending_sectors > 0) {
		if (VSync(-1) > _read_timeout) {
			if (_poll_retry() < 0)
				return -1;
		}

		//if (CdSync(1, 0) == CdlDiskError)
			//return -1;
	}

	CdlIntrResult status = CdSync(0, result);
	if (_pending_sectors < 0)
		return -2;
	if (status != CdlComplete)
		return -1;

	return 0;
}

int CdReadRetry(int sectors, uint32_t *buf, int mode, int attempts) {

	if (CdReadSync(1, 0) > 0) {
		return 0;
	}

	_read_addr        = buf;
	_read_timeout     = VSync(-1) + CD_READ_TIMEOUT;
	_pending_attempts = attempts - 1;
	_pending_sectors  = sectors;
	_total_sectors    = sectors;
	_sector_size      = (mode & CdlModeSize) ? 585 : 512;

	disableInterrupts();
	_cd_override_callback = &_sector_callback;
	enableInterrupts();

	uint8_t _mode = mode;
	if (!CdCommand(CdlSetmode, &_mode, 1, 0))
		return 0;
	if (!CdCommand(CdlReadN, 0, 0, _read_result))
		return 0;

	return 1;
}

static int _CdReadIsoDescriptor(int session_offs)
{
	int i;
	CdlLOC loc;
	ISO_DESCRIPTOR *descriptor;
	
	// Check if the lid had been opened
	if( !_cd_media_changed )
	{
		CdControl(CdlNop, 0, 0);
		if( (CdStatus()&0x10) )
		{
			// Check if lid is still open
			CdControl(CdlNop, 0, 0);
			if( (CdStatus()&0x10) )
			{
				_cd_iso_error = CdlIsoLidOpen;
				return -1;
			}
			// Reparse the file system
			_cd_media_changed = 1;
		}
	}
	
	if( !_cd_media_changed )
	{
		return 0;
	}

	// Seek to volume descriptor
	CdIntToPos(16+session_offs, &loc);
	if( !CdControl(CdlSetloc, (uint8_t*)&loc, 0) )
	{
		_cd_iso_error = CdlIsoSeekError;
		return -1;
	}

	// Read volume descriptor
	CdReadRetry(1, (uint32_t*)_cd_iso_descriptor_buff, CdlModeSpeed, CD_READ_ATTEMPTS);
	
	if( CdReadSync(0, 0) )
	{
		_cd_iso_error = CdlIsoReadError;
		return -1;
	}

	// Verify if volume descriptor is present
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	if( memcmp("CD001", descriptor->header.id, 5) )
	{
		_cd_iso_error = CdlIsoInvalidFs;
		return -1;
	}

	// Allocate path table buffer
	i = ((2047+descriptor->pathTableSize.lsb)>>11)<<11;
	if( _cd_iso_pathtable_buff )
	{
		free(_cd_iso_pathtable_buff);
	}
	_cd_iso_pathtable_buff = (uint8_t*)malloc(i);

	// Read path table
	CdIntToPos(descriptor->pathTable1Offs, &loc);
	CdControl(CdlSetloc, (uint8_t*)&loc, 0);
	CdReadRetry(i>>11, (uint32_t*)_cd_iso_pathtable_buff, CdlModeSpeed, CD_READ_ATTEMPTS);
	if( CdReadSync(0, 0) )
	{
		_cd_iso_error = CdlIsoReadError;
		return -1;
	}
	
	_cd_iso_last_dir_lba	= 0;
	_cd_iso_error			= CdlIsoOkay;
	
	_cd_media_changed		= 0;
	
	return 0;
}


static int get_pathtable_entry(int entry, ISO_PATHTABLE_ENTRY *tbl, char *namebuff)
{
	int i;
	uint8_t *tbl_pos;
	ISO_PATHTABLE_ENTRY *tbl_entry;
	ISO_DESCRIPTOR *descriptor;
	
	descriptor = (ISO_DESCRIPTOR*)_cd_iso_descriptor_buff;
	
	tbl_pos = _cd_iso_pathtable_buff;
	tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
	
	i = 0;
	while( (int)(tbl_pos-_cd_iso_pathtable_buff) <
		descriptor->pathTableSize.lsb )
	{
		if( i == (entry-1) )
		{
			if( namebuff )
			{
				memcpy(
					namebuff, 
					tbl_pos+sizeof(ISO_PATHTABLE_ENTRY), 
					tbl_entry->nameLength
				);
				namebuff[tbl_entry->nameLength] = 0;
			}
			
			if( tbl )
			{
				*tbl = *tbl_entry;
			}
			
			return 0;
		}
		
		// Advance to next entry
		tbl_pos += sizeof(ISO_PATHTABLE_ENTRY)
			+(2*((tbl_entry->nameLength+1)/2));
			
		tbl_entry = (ISO_PATHTABLE_ENTRY*)tbl_pos;
		i++;
	}
	
	if( entry <= 0 )
	{
		return i+1;
	}
	
	return -1;
}

static char* get_pathname(char *path, const char *filename)
{
	const char *c = 0;
	for (const char *i = filename; *i; i++) {
		if (IS_PATH_SEP(*i))
			c = i;
	}
	
	if(( c == filename ) || ( !c ))
	{
		path[0] = DEFAULT_PATH_SEP;
		path[1] = 0;
		return NULL;
	}
	
	memcpy(path, filename, c - filename);
	path[c - filename] = 0;
	return path;
}

static char* resolve_pathtable_path(int entry, char *rbuff)
{
	char namebuff[16];
	ISO_PATHTABLE_ENTRY tbl_entry;
	
	*rbuff = 0;
	
	do
	{
		if( get_pathtable_entry(entry, &tbl_entry, namebuff) )
		{
			return NULL;
		}
		
		rbuff -= tbl_entry.nameLength;
		memcpy(rbuff, namebuff, tbl_entry.nameLength);
		rbuff--;
		*rbuff = DEFAULT_PATH_SEP;
	
		// Parse to the parent
		entry = tbl_entry.dirLevel;
	
	} while( entry > 1 );
	
	return rbuff;
}

static int _CdReadIsoDirectory(int lba)
{
	int i;
	CdlLOC loc;
	ISO_DIR_ENTRY *direntry;
	
	if( lba == _cd_iso_last_dir_lba )
	{
		return 0;
	}
	
	CdIntToPos(lba, &loc);
	i = CdPosToInt(&loc);

	if( !CdControl(CdlSetloc, (uint8_t*)&loc, 0) )
	{
		_cd_iso_error = CdlIsoSeekError;
		return -1;
	}
	
	if( _cd_iso_directory_buff )
	{
		free(_cd_iso_directory_buff);
	}
	
	_cd_iso_directory_buff = (uint8_t*)malloc(2048);
	CdReadRetry(1, (uint32_t*)_cd_iso_directory_buff, CdlModeSpeed, CD_READ_ATTEMPTS);
	if( CdReadSync(0, 0) )
	{
		_cd_iso_error = CdlIsoReadError;
		return -1;
	}
	
	direntry = (ISO_DIR_ENTRY*)_cd_iso_directory_buff;
	_cd_iso_directory_len = direntry->entrySize.lsb;

	if( _cd_iso_directory_len > 2048 )
	{
		if( !CdControl(CdlSetloc, (uint8_t*)&loc, 0) )
		{
			_cd_iso_error = CdlIsoSeekError;
			return -1;
		}
	
		free(_cd_iso_directory_buff);
		i = ((2047+_cd_iso_directory_len)>>11)<<11;
		_cd_iso_directory_buff = (uint8_t*)malloc(i);

		CdReadRetry(i>>11, (uint32_t*)_cd_iso_directory_buff, CdlModeSpeed, CD_READ_ATTEMPTS);
		if( CdReadSync(0, 0) )
		{
			_cd_iso_error = CdlIsoReadError;
			return -1;
		}
	}
	
	_cd_iso_last_dir_lba = lba;
	_cd_iso_error = CdlIsoOkay;
	
	return 0;
}

static char* get_filename(char *name, const char *filename)
{
	const char *c = 0;
	for (const char *i = filename; *i; i++) {
		if (IS_PATH_SEP(*i))
			c = i;
	}
	
	if (!c) {
		strcpy(name, filename);
		return name;
	}
	if (c == filename) {
		strcpy(name, filename+1);
		return name;
	}
	
	c++;
	strcpy(name, c);
	return name;
}

static int find_dir_entry(const char *name, ISO_DIR_ENTRY *dirent)
{
	int i;
	int dir_pos;
	ISO_DIR_ENTRY *dir_entry;
	char namebuff[16];

	i = 0;
	dir_pos = 0;
	while(dir_pos < _cd_iso_directory_len)
	{
		dir_entry = (ISO_DIR_ENTRY*)(_cd_iso_directory_buff+dir_pos);

		if( !(dir_entry->flags & 0x2) )
		{
			memcpy(
				namebuff, 
				_cd_iso_directory_buff+dir_pos+sizeof(ISO_DIR_ENTRY), 
				dir_entry->identifierLen
			);
			namebuff[dir_entry->identifierLen] = 0;
			
			if( strcmp(namebuff, name) == 0 )
			{
				*dirent = *dir_entry;
				return 0;
			}
		}
		
		dir_pos += dir_entry->entryLength;
		i++;
		
		// Check if padding is reached (end of record sector)
		if( _cd_iso_directory_buff[dir_pos] == 0 )
		{
			// Snap it to next sector
			dir_pos = ((dir_pos+2047)>>11)<<11;
			
		}
	}
	
	return -1;
}

CdlFILE *CdSearchFile(CdlFILE *fp, const char *filename)
{
	int i,j,found_dir,num_dirs;
	int dir_len;
	char tpath_rbuff[128];
	char search_path[128];
	char *rbuff;
	ISO_PATHTABLE_ENTRY tbl_entry;
	ISO_DIR_ENTRY dir_entry;
	
	// Read ISO descriptor if changed flag is set
	//if( _cd_media_changed )
	//{
		// Read ISO descriptor and path table
	if( _CdReadIsoDescriptor(0) )
	{
		return NULL;
	}

	//	_sdk_log("ISO file system cache updated.\n");
	//	_cd_media_changed = 0;
	//}
	
	// Get number of directories in path table
	num_dirs = get_pathtable_entry(0, NULL, NULL);
	
	
	if( get_pathname(search_path, filename) )
	{
		//_sdk_log("Search path = %s\n", search_path);
	}
	
	found_dir = 0;
	for(i=1; i<num_dirs; i++)
	{
		rbuff = resolve_pathtable_path(i, tpath_rbuff+127);

		if( rbuff )
		{
			if( strcmp(search_path, rbuff) == 0 )
			{
				found_dir = i;
				break;
			}
		}
	}
	
	if( !found_dir )
	{
		return NULL;
	}


	get_pathtable_entry(found_dir, &tbl_entry, NULL);

	_CdReadIsoDirectory(tbl_entry.dirOffs);
	get_filename(fp->name, filename);
	
	// Add version number if not specified
	if( !strchr(fp->name, ';') )
	{
		strcat(fp->name, ";1");
	}
	
	
	if( find_dir_entry(fp->name, &dir_entry) )
	{
		return NULL;
	}

	CdIntToPos(dir_entry.entryOffs.lsb, &fp->pos);
	fp->size = dir_entry.entrySize.lsb;
	
	return fp;
}