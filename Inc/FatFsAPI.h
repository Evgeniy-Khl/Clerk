#ifndef __FATFSAPI_H__
#define __FATFSAPI_H__

#include "integer.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
#include "ff_gen_drv.h"
#include "fatfs.h"
#include "sd.h"

#define LEN_BUFF       160
//--- _FS_NORTC	1  � "ffconf.h"

uint8_t My_LinkDriver(void);
DRESULT SD_write (const TCHAR* flname);
void SD_close(void);
DRESULT SD_dir (void);
void SD_CleanUp(void);

#endif // __FATFSAPI_H__

