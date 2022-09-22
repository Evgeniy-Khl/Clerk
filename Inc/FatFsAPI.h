#ifndef __FATFSAPI_H__
#define __FATFSAPI_H__

#include "tft_proc.h"
#include "ff_gen_drv.h"
#include "fatfs.h"
#include "sd.h"

#define LEN_BUFF       160
//--- _FS_NORTC	1  â "ffconf.h"

uint8_t My_LinkDriver(void);
DRESULT SD_write (const TCHAR* flname);
DRESULT SD_dir (void);

#endif // __FATFSAPI_H__

