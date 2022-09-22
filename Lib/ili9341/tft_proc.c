#include "my.h"
#include "rtc.h"
#include "tft_proc.h"
#include "ili9341_touch.h"

extern int16_t set[MAX_SET], newval[MAX_SET];
extern uint8_t displ_num, newButt, ticTimer, ticTouch, show, Y_txt, X_left, Y_top, Y_bottom, buttonAmount, secTick, card;
extern int8_t ds18b20_amount, numSet, numDate, newDate, tiimeDispl;
extern uint16_t fillScreen;
extern int16_t ds18b20_val[];
extern struct ram_structure {int x,y; char w,h;} buttons[];
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

void TFT_init(){
  ILI9341_Unselect();
  ILI9341_TouchUnselect();
  ILI9341_Init();
  ILI9341_FillScreen(fillScreen);
  Y_txt = 5; X_left = 5;
  ILI9341_WriteString(45, Y_txt, "Клерк v 0.1", Font_11x18, ILI9341_YELLOW, fillScreen);
  Y_txt = Y_txt+18+5;
//  ILI9341_WriteString(25, Y_txt, "прибор корректировки", Font_11x18, ILI9341_GREEN, fillScreen);
//  Y_txt = Y_txt+18+5;
//  ILI9341_WriteString(25, Y_txt, "точности измерений", Font_11x18, ILI9341_GREEN, fillScreen);
//  Y_txt = Y_txt+18+5;
//  ILI9341_WriteString(25, Y_txt, "датчиков температуры.", Font_11x18, ILI9341_GREEN, fillScreen);
//  HAL_Delay(5000);
}

void WindowDraw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t setcolor, const char* str){
 uint16_t strColor, bordColor; 
  switch (setcolor)
  {
  	case ILI9341_BLACK: strColor=ILI9341_WHITE; bordColor = ILI9341_WHITE; break;
    case ILI9341_BLUE:  strColor=ILI9341_WHITE; bordColor = ILI9341_WHITE; break;
  	default: strColor=ILI9341_BLACK; bordColor = ILI9341_BLACK; break;
  }
  ILI9341_FillRectangle(x, y, w, h, setcolor);
  for(int i = x; i < x+w+1; i++){
     ILI9341_DrawPixel(i, y-1, bordColor);
     ILI9341_DrawPixel(i, y+h+1, bordColor);
  }
  for(int i = y; i < y+h+1; i++){
     ILI9341_DrawPixel(x-1, i, bordColor);
     ILI9341_DrawPixel(x+w+1, i, bordColor);
  }
  ILI9341_WriteString(x+(w-11)/2, y+(h-18)/2, str, Font_11x18, strColor, setcolor);
}

void initializeButtons(uint8_t col, uint8_t row, uint8_t h)// высота кнопки
{
  uint8_t i,j,indx;
  uint16_t x, y, w;
  switch (col)                  // ширина кнопки зависит от кол-ва кнопок в строке
   {
    case 4: w = 72; break;
    case 3: w = 100; break;
    case 2: w = 150; break;   
    default: w = ILI9341_WIDTH-6;
   };
  if(h<20) h=20;
  y = ILI9341_HEIGHT - h - 4;      // верхний контур кнопки
  indx = 0;
  for (j=0; j<row; j++)
   {
    x = 4;// начало 1 кнопки
    for (i=0; i<col; i++)
      {
        buttons[indx].x = x+i*(w+8);// интервал между кнопками по горизонтали
        buttons[indx].w = w;
        buttons[indx].h = h;
        buttons[indx].y = y;
        indx++;
      }
    y -= (h*(row-1)+4);// интервал между кнопками по вертикали
   }
  Y_bottom = y;// нижняя граница до которой можно закрашивать экран
  buttonAmount = col * row;// обшее количество кнопок
}
//-------------------- цвет фона --- цвет рамки -- цвет текста --- номер --- текст ---------
void drawButton(uint16_t setcolor, uint8_t b, char *str)
{
  uint16_t x, y, w, h;
  w = buttons[b].w;      // ширина кнопки
  h = buttons[b].h;      // высота кнопки
  x = buttons[b].x;      // начало контура кнопки
  y = buttons[b].y;      // начало контура кнопки

  ILI9341_FillRectangle(x, y, w, h, setcolor);

  uint16_t strColor, bordColor;
  if (fillScreen == ILI9341_BLACK) bordColor=ILI9341_WHITE;
  else bordColor=ILI9341_BLACK;
  switch (setcolor){
  	case ILI9341_BLACK: strColor=ILI9341_WHITE; break;
    case ILI9341_BLUE:  strColor=ILI9341_WHITE; break;
  	default: strColor=ILI9341_BLACK; break;
  }
  ILI9341_FillRectangle(x, y, w, h, setcolor);
  for(uint16_t i = x; i < x+w+1; i++){
     ILI9341_DrawPixel(i, y-1, bordColor);
     ILI9341_DrawPixel(i, y+h+1, bordColor);
  }
  for(uint16_t i = y; i < y+h+1; i++){
     ILI9341_DrawPixel(x-1, i, bordColor);
     ILI9341_DrawPixel(x+w+1, i, bordColor);
  }

  x = x + w/2 - strlen(str)*11/2;  // ширина символа 11
  y = y + h/2 - 9;                 // высота символа 18 / 2
  
  ILI9341_WriteString(x, y, str, Font_11x18, strColor, setcolor);
//  sprintf(buffTFT,"sizeof=%d",strlen(str));
//  ILI9341_WriteString(5, ILI9341_HEIGHT-(45+18+5+(18+5)*b), buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK); 
}

// проверка попадания пересчитаной координаты в область кнопки.
uint8_t contains(uint16_t touch_X, uint16_t touch_Y, uint8_t b){
 uint16_t beg, end;
   beg = buttons[b].x;
   end = beg + buttons[b].w-3;
   if ((touch_X < beg)||(touch_X > end)) return 0;
   beg = buttons[b].y;
   end = beg + buttons[b].h-3;
   if ((touch_Y < beg)||(touch_Y > end)) return 0;
   else ticTouch = 30;
   return 1;
 }

void checkButtons(uint8_t item){
    switch (displ_num){
      case 0://--------- температуры всех датчиков ----------------------
        switch (item){
          case 0: displ_num = 1; newButt = 1; break;
          case 1: displ_num = 4; newButt = 1; break;
          case 2: displ_num = 2; newButt = 1; break;
        }
        item = 10;
        break;
      case 1://--------- информация о файле SD ----------------------------------
        switch (item){
          case 0: displ_num = 0; newButt = 1; break;
          case 1: displ_num = 0; newButt = 1; break;
        }
        item = 10;
        break;
      case 2://--------- Установки ----------------------------------
        switch (item){
          case 0: displ_num = 0; newButt = 1; break;
          case 1: if (--numSet<0) numSet = 0;	break;
          case 2: if (++numSet>4) numSet = 4;	break;
          case 3: newval[numSet] = set[numSet]; displ_num = 3; newButt = 1; break;
        }
        item = 10;
        break;
      case 3://--------- корекция Установки -------------------------
        switch (item){
          case 0: displ_num = 2; newButt = 1; break;
          case 1: ++newval[numSet];	break;
          case 2: --newval[numSet];	break;
          case 3: 
            ILI9341_FillRectangle(0, Y_top, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
            ILI9341_WriteString(45, Y_top+40, "ВЫПОЛНЯЮ  ЗАПИСЬ!", Font_11x18, ILI9341_GREEN, ILI9341_BLACK);
            set[numSet] = newval[numSet];
            writeSetToBackup(RTC_BKP_DR2);                    // запишем новые значения установок
            HAL_Delay(1000);
            displ_num = 2; newButt = 1; break;
        }
        item = 10;
        break;
      case 4://--------- Дата и Время ----------------------------------
        switch (item){
          case 0: displ_num = 0; newButt = 1; break;
          case 1: if (--numDate<0) numDate = 0;	break;
          case 2: if (++numDate>4) numDate = 4;	break;
          case 3: /*newval[numSet] = set[numSet]; */ 
            switch (numDate){
              case 0: newDate = sDate.Year; break;
              case 1: newDate = sDate.Month; break;
              case 2: newDate = sDate.Date; break;
              case 3: newDate = sTime.Hours; break;
              case 4: newDate = sTime.Minutes; break;
            }
            displ_num = 5; newButt = 1;
            break;
        }
        item = 10;
        break;
      case 5://--------- корекция Дата и Время -------------------------
        switch (item){
          case 0: displ_num = 4; newButt = 1; break;
          case 1: if (++newDate>59) newDate = 0;	break;
          case 2: if (--newDate<0) newDate = 59;	break;
          case 3: 
            ILI9341_FillRectangle(0, Y_top, ILI9341_WIDTH, ILI9341_HEIGHT, fillScreen);
            ILI9341_WriteString(45, Y_top+40, "ВЫПОЛНЯЮ  ЗАПИСЬ!", Font_11x18, ILI9341_GREEN, ILI9341_BLACK);
            switch (numDate){
              case 0: sDate.Year = newDate; break;
              case 1: if(newDate>12) newDate = 12; sDate.Month = newDate; break;
              case 2: if(newDate>31) newDate = 31; sDate.Date  = newDate; break;
              case 3: if(newDate>23) newDate = 23; sTime.Hours = newDate; break;
              case 4: sTime.Minutes = newDate; break;
            }
            if (numDate==4) sTime.Seconds = 0;
            if (numDate<3){
              Fix_Date(&sDate);
              if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK){
                ILI9341_WriteString(45, Y_top+63, "ОШИБКА  ЗАПИСИ!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
              }
              writeDateToBackup(RTC_BKP_DR1);
            }
            else {
              if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK){
                ILI9341_WriteString(45, Y_top+63, "ОШИБКА  ЗАПИСИ!", Font_11x18, ILI9341_MAGENTA, ILI9341_BLACK);
              }
            }
            HAL_Delay(1000);
            displ_num = 4; newButt = 1; break;
        }
        item = 10;
        break;
    }
}
