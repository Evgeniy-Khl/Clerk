/* vim: set ai et ts=4 sw=4: */
#ifndef __ILI9341_TOUCH_H__
#define __ILI9341_TOUCH_H__

#include "main.h"
#include <stdbool.h>

/*** Redefine if necessary ***/

// Warning! Use SPI bus with < 1.3 Mbit speed, better ~650 Kbit to be save.
#define ILI9341_TOUCH_SPI_PORT hspi2
extern SPI_HandleTypeDef ILI9341_TOUCH_SPI_PORT;

#define ILI9341_TOUCH_IRQ_Pin       T_IRQ_Pin
#define ILI9341_TOUCH_IRQ_GPIO_Port T_IRQ_GPIO_Port
#define ILI9341_TOUCH_CS_Pin        T_CS_Pin
#define ILI9341_TOUCH_CS_GPIO_Port  T_CS_GPIO_Port

// change depending on screen orientation
#define ILI9341_TOUCH_SCALE_X 320
#define ILI9341_TOUCH_SCALE_Y 240

// to calibrate uncomment UART_Printf line in ili9341_touch.c
#define ILI9341_TOUCH_MIN_RAW_X 1500
#define ILI9341_TOUCH_MAX_RAW_X 29000
#define ILI9341_TOUCH_MIN_RAW_Y 2000
#define ILI9341_TOUCH_MAX_RAW_Y 30000

// call before initializing any SPI devices
void ILI9341_TouchUnselect(void);
void ILI9341_TouchSelect(void);
bool ILI9341_TouchPressed(void);
bool ILI9341_TouchGetCoordinates(uint16_t* x, uint16_t* y);

#endif // __ILI9341_TOUCH_H__
