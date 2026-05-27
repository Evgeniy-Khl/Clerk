#include "FatFsAPI.h"
#include "my.h"
#include "lang.h"
#include <stdio.h>
#include "ili9341.h"
#include "fonts.h"
#include "rtc.h"

extern void Error_Handler(void);
extern char USERPath[]; /* logical drive path */
extern char fileName[];
extern char txt[];
FATFS SDFatFs;
FRESULT res;
DWORD fre_clust, fre_sect, tot_sect;
FATFS *fs;
FIL MyFile;
FILINFO fileInfo;
DIR dir;
union sd {uint8_t sect[512]; char buffer2[512];} buffer;

extern uint8_t Y_txt, X_left, displ_num, ds18b20_amount, checkButt, Y_bottom, card;
extern volatile uint8_t newButt;
extern char buffTFT[], txt[];
extern uint16_t fillScreen;
extern int16_t ds18b20_val[], set[], touch_x;
extern RTC_DateTypeDef sDate;
extern RTC_TimeTypeDef sTime;
uint32_t bwrt;

//-- My_LinkDriver --------------------------------------------------------------
// Mounts SD card and opens the log file. Keeps the file open for efficiency.
uint8_t My_LinkDriver(void){
  uint8_t cardOk=0;
  FRESULT fr;

  // Unlink/Link driver to reset state
  FATFS_UnLinkDriver(USERPath);
  if (FATFS_LinkDriver(&USER_Driver, USERPath) != 0) return 0;

  // Mount the drive
  if(f_mount(&SDFatFs, (const char*)USERPath, 1) != FR_OK) return 0;

  // Open existing or create new file
  fr = f_open(&MyFile, fileName, FA_OPEN_ALWAYS | FA_WRITE);

  if (fr == FR_OK) {
    // If file was just created (size is 0), write header
    if (f_size(&MyFile) == 0) {
      buffTFT[0] = 0;
      strcat(buffTFT, "DateTime");
      for(uint8_t i=0; i<ds18b20_amount; i++){
        sprintf(txt, ";t%u", i+1); strcat(buffTFT, txt);
      }
      for(uint8_t i=0; i<MAX_SET; i++){
        sprintf(txt, ";set%u", i+1); strcat(buffTFT, txt);
      }
      strcat(buffTFT, "\r\n");

      f_write(&MyFile, buffTFT, strlen(buffTFT), (void*)&bwrt);
      f_sync(&MyFile);
      ILI9341_WriteString(X_left, Y_bottom - 22, (char*)STR_NEW_LOG, Font_11x18, ILI9341_GREEN, fillScreen);    
    } else {
      // Seek to end for appending
      f_lseek(&MyFile, f_size(&MyFile));
      ILI9341_WriteString(X_left, Y_bottom - 22, (char*)STR_LOG_APPEND, Font_11x18, ILI9341_GREEN, fillScreen); 
    }
    cardOk = 1;
  } else {
    ILI9341_WriteString(X_left, Y_bottom - 22, (char*)STR_FILE_ERROR, Font_11x18, ILI9341_YELLOW, ILI9341_RED); 
    f_mount(NULL, (const TCHAR*)USERPath, 0);
  }

  return cardOk;
}

//-- SD_write --------------------------------------------------------------------------------------------------
// Appends data to the already open log file and syncs it.
DRESULT SD_write (const char* flname){
  uint8_t i;
  if (!card) return RES_NOTRDY;

  // Human readable date: 2026-05-25 12:00:00
  sprintf(buffTFT, "20%02u-%02u-%02u %02u:%02u:%02u", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);
  for(i=0; i<ds18b20_amount; i++) {
    sprintf(txt, ";%.1f", (float)ds18b20_val[i]/10);
    strcat(buffTFT, txt);
  }
  for(i=0; i<MAX_SET; i++) {
    if (i<2) sprintf(txt, ";%.1f", (float)set[i]/10);
    else sprintf(txt, ";%i", set[i]);
    strcat(buffTFT, txt);
  }
  strcat(buffTFT, "\r\n");

  res = f_write(&MyFile, buffTFT, strlen(buffTFT), (void*)&bwrt);

  if (res == FR_OK && bwrt > 0) {
    f_sync(&MyFile); // Flush data to physical media
    if (displ_num == 0) {
        ILI9341_WriteString(X_left, Y_bottom - 22, (char*)STR_SAVE_DATA, Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
    }
    return RES_OK;
  } else {
    ILI9341_WriteString(X_left, Y_bottom - 22, (char*)STR_WRT_ERROR, Font_11x18, ILI9341_YELLOW, ILI9341_RED);  
    card = 0; // Mark card as invalid
    return RES_ERROR;
  }
}

//-- SD_close ------------------------------------------------------------------------------------------------- 
void SD_close(void) {
    if (card) {
        f_close(&MyFile);
        f_mount(NULL, (const TCHAR*)USERPath, 0);
        card = 0;
    }
}

//-- read dir ------------------------------------------------------------------------------------------------- 

DRESULT SD_dir (void){
  uint8_t item;
  sprintf(buffTFT, "File  : %s", fileName);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, fillScreen);
  Y_txt = Y_txt + 18 + 5;

  sprintf(buffTFT, "Size: %u bytes", f_size(&MyFile));
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, fillScreen);
  Y_txt = Y_txt + 18 + 5;

  if (card){
      fileInfo.lfname = (char*)buffer.sect;
      fileInfo.lfsize = sizeof(buffer.sect);
      res = f_opendir(&dir, "/");
      if (res == FR_OK) {
        item = 0;
        while(1) {
          res = f_readdir(&dir, &fileInfo);
          if (res == FR_OK && fileInfo.fname[0]) {
            char* fn = fileInfo.lfname;
            if(strlen(fn)) {
              ILI9341_WriteString(X_left, Y_txt, fn, Font_7x10, ILI9341_WHITE, fillScreen);
              item++;
            }
            else {
              ILI9341_WriteString(X_left, Y_txt, fileInfo.fname, Font_7x10, ILI9341_WHITE, fillScreen);
              item++;
            }
            if(fileInfo.fattrib & AM_DIR) {
              ILI9341_WriteString(X_left + 80, Y_txt, "[DIR]", Font_7x10, ILI9341_MAGENTA, fillScreen);
            }
          }
          else break;

          if (item % 3 == 0){
            Y_txt = Y_txt + 12;
            X_left = 5;
          }
          else X_left = X_left + 105;
        }
        f_closedir(&dir);
      }
  }
  return RES_OK;
}

//-- SD_CleanUp -------------------------------------------------------------------------------------------------
// Deletes files older than set[4] months.
// Files are named YY_MM_DD.csv
void SD_CleanUp(void) {
    DIR dj;
    FILINFO fno;
    FRESULT fr;
    int32_t file_date_val, current_date_val;
    int32_t diff_months;

    if (!card) return;

    current_date_val = (int32_t)sDate.Year * 12 + sDate.Month;

    fr = f_opendir(&dj, "/");
    if (fr == FR_OK) {
        for (;;) {
            fr = f_readdir(&dj, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) continue;

            // Check if file name matches YY_MM_DD.csv (length 12)
            if (strlen(fno.fname) == 12 && fno.fname[2] == '_' && fno.fname[5] == '_' && strstr(fno.fname, ".csv")) {
                int y, m, d;
                if (sscanf(fno.fname, "%02u_%02u_%02u.csv", &y, &m, &d) == 3) {
                    file_date_val = (int32_t)y * 12 + m;
                    diff_months = current_date_val - file_date_val;

                    if (diff_months >= set[4]) {
                        f_unlink(fno.fname);
                    }
                }
            }
        }
        f_closedir(&dj);
    }
}
