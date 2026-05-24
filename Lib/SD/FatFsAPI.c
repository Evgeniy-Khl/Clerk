#include "FatFsAPI.h"
#include "my.h"
#include <stdio.h>
#include "ili9341.h"
#include "fonts.h"
#include "rtc.h"

extern void Error_Handler(void);
extern char USERPath[]; /* logical drive path */
extern char fileName[];
extern char txt[];
FATFS SDFatFs;
FRESULT res; //��������� ����������
DWORD fre_clust, fre_sect, tot_sect;
FATFS *fs;
FIL MyFile;
FILINFO fileInfo;
DIR dir;
union sd {uint8_t sect[512]; char buffer2[512];} buffer;
//uint8_t result;
extern uint8_t Y_txt, X_left, displ_num, ds18b20_amount, checkButt, Y_bottom, newButt, card;
extern char buffTFT[], txt[];
extern uint16_t fillScreen;
extern int16_t ds18b20_val[], set[], touch_x;
extern uint32_t UnixTime;
extern RTC_DateTypeDef sDate;
uint32_t bwrt;   // ��� �������� �� ������� ���������� ������� ���������� ����

//-- My_LinkDriver --------------------------------------------------------------
uint8_t My_LinkDriver(void){
 uint8_t cardOk=0, res, item;
  FATFS_LinkDriver(&USER_Driver, USERPath);
  if(f_mount(&SDFatFs,(const char*)USERPath,0)!=FR_OK) {
    ILI9341_WriteString(X_left, Y_bottom - 22, "�� ����������� SD �����!", Font_7x10, ILI9341_MAGENTA, fillScreen);   
    HAL_Delay(3000);
  }
  else {
    item = f_open(&MyFile, fileName, FA_OPEN_EXISTING|FA_WRITE);
    switch (item)
    {
    	case FR_NOT_READY:      // (3) The physical drive cannot work
        ILI9341_WriteString(X_left, Y_bottom - 22, "   SD ����� �� ���������!   ", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
    		break;
    	case FR_DISK_ERR:       // (1) A hard error occurred in the low level disk I/O layer
        ILI9341_WriteString(X_left, Y_bottom - 22, "������ SD �����!", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
    		break;
      case FR_EXIST:          // SD ����� ��������� � ���� ��� ���� �� �����.
        sprintf(buffTFT,"���� ");
        strcat(buffTFT,fileName);
        strcat(buffTFT," ��� ����!");
        ILI9341_WriteString(X_left, Y_bottom - 22, buffTFT, Font_11x18, ILI9341_GREEN, fillScreen);
        f_close(&MyFile); cardOk = 1;
        break;
      case FR_NO_FILE:        // (4) Could not find the file
        // ���� ����� ��� �� ��������� ������ ������...
        if(f_open(&MyFile, fileName, FA_CREATE_NEW|FA_WRITE)!=FR_OK) {
          ILI9341_WriteString(X_left, Y_bottom - 22, "�� ���� ������� ����!", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
        }
        else {
          sprintf(buffTFT,"timeStamp;");
          for(item=0; item<ds18b20_amount; item++){
            sprintf(txt,"t%u;",item+1); strcat(buffTFT,txt);
          }
          for(item=0; item<MAX_SET; item++){
            sprintf(txt,"set%u;",item+1); strcat(buffTFT,txt);
          }
          strcat(txt,"\r\n"); strcat(buffTFT,txt);
          for(item=0; item<LEN_BUFF; item++){
            if (buffTFT[item]==0) break;
          }
          res = f_write(&MyFile, buffTFT, item, (void*)&bwrt);
          if((bwrt == 0)||(res!=FR_OK))	{
            ILI9341_WriteString(X_left, Y_bottom - 22, "������ �������� ���������!", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
          }
          else {
            ILI9341_WriteString(X_left, Y_bottom - 22, "����� ���� ������!", Font_11x18, ILI9341_GREEN, fillScreen);
            f_close(&MyFile); cardOk = 1;
          }
        }
        break;
      case FR_OK:
//        sprintf(buffTFT,"SD ����� ����������: 0x%02X",sdinfo.type);
        ILI9341_WriteString(X_left, Y_bottom - 22, "SD ����� Ok!", Font_11x18, ILI9341_GREEN, fillScreen);
        cardOk = 1;
        break;
    	default:
        sprintf(buffTFT,"f_open: %u",item);
        ILI9341_WriteString(X_left, Y_bottom - 22, buffTFT, Font_11x18, ILI9341_YELLOW, fillScreen);
    		break;
    }
    f_mount(NULL,(TCHAR const*)USERPath,0);
  }
  if (cardOk==0) FATFS_UnLinkDriver(USERPath);
  return cardOk;
}

  //write ----------------------------------------------------------------------------------------------------
DRESULT SD_write (const char* flname){
 uint8_t i;
  if(f_mount(&SDFatFs,(const char*)USERPath,0)!=FR_OK) {
    ILI9341_WriteString(X_left, Y_bottom - 22, "�� ����������� SD �����!", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
  }
  else {
    if(f_open(&MyFile, flname, FA_OPEN_EXISTING|FA_WRITE)!=FR_OK) {
      ILI9341_WriteString(X_left, Y_bottom - 22, "������ ������� ����!", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
      card = 0; newButt = 1; FATFS_UnLinkDriver(USERPath);
    }
    else {
      uint32_t f_size = MyFile.fsize;
      res = f_lseek(&MyFile, f_size);
      if (res!=FR_OK){
        ILI9341_WriteString(X_left, Y_bottom - 22, "������ SD �����!", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
      }
      else {
        sprintf(buffTFT,"%u;", UnixTime);
        for(i=0;i<ds18b20_amount;i++) {
          sprintf(txt,"%.1f;", (float)ds18b20_val[i]/10);
          strcat(buffTFT,txt);
        }
        for(i=0;i<MAX_SET;i++) {
          if (i<2) sprintf(txt,"%.1f;", (float)set[i]/10);
          else sprintf(txt,"%i;", set[i]);
          strcat(buffTFT,txt);
        }
        strcat(buffTFT,"\r\n");
        for(i=0;i<LEN_BUFF;i++) {
          if (buffTFT[i]==0) break;
        }
        res = f_write(&MyFile, buffTFT, i,(void*)&bwrt);
        if((bwrt==0)||(res!=FR_OK)) {
          ILI9341_WriteString(X_left, Y_bottom - 22, "������ ������� ������!", Font_11x18, ILI9341_YELLOW, ILI9341_RED);
        }
      }
      f_close(&MyFile);
      f_mount(NULL,(TCHAR const*)USERPath,0);
      if (displ_num==0) ILI9341_WriteString(X_left, Y_bottom - 22, " ������� ", Font_11x18, ILI9341_BLACK, ILI9341_GREEN);
    }
  }
  return RES_OK;
}

  //read dir -------------------------------------------------------------------------------------------------
DRESULT SD_dir (void){
  uint8_t item;
  sprintf(buffTFT,"����  : ");
  strcat(buffTFT,fileName);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, fillScreen);
  Y_txt = Y_txt+18+5;
	sprintf(buffTFT,"������: %u byte",MyFile.fsize);
  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, fillScreen);
  Y_txt = Y_txt+18+5;
//  sprintf(buffTFT,"����� : %u �����.",allRecods);
//  ILI9341_WriteString(X_left, Y_txt, buffTFT, Font_11x18, ILI9341_WHITE, fillScreen);
//  Y_txt = Y_txt+18+5;
  if (card){
    if(f_mount(&SDFatFs,(TCHAR const*)USERPath,0)!=FR_OK) {
      Error_Handler();
    }
    else {
      fileInfo.lfname = (char*)buffer.sect;
      fileInfo.lfsize = sizeof(buffer.sect);
      res = f_opendir(&dir, "/");
      if (res == FR_OK) {
        item = 0;
        while(1) {
          res = f_readdir(&dir, &fileInfo);
          if (res==FR_OK && fileInfo.fname[0]) {
            char* fn = fileInfo.lfname;
            if(strlen(fn)) {
              ILI9341_WriteString(X_left, Y_txt, fn, Font_7x10, ILI9341_WHITE, fillScreen);
              item++;
            }
            else {
              ILI9341_WriteString(X_left, Y_txt, fileInfo.fname, Font_7x10, ILI9341_WHITE, fillScreen);
              item++;
              if(fileInfo.fattrib&AM_DIR) {
                ILI9341_WriteString(X_left, Y_txt, " [DIR]  ", Font_7x10, ILI9341_MAGENTA, fillScreen);
                item++;
              }
            }
          }
          else break;
          if (item==2||item==5||item==8||item==11||item==14||item==17||item==20){
            Y_txt = Y_txt+12;
            X_left = 5;
          }
          else X_left = X_left + 105;
        }
        f_closedir(&dir);
      }
    }
  }
  return RES_OK;
}
  //----------------------------------------------------------------------------------------------------------
