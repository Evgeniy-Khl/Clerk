#ifndef TFT_PROC_H_
#define TFT_PROC_H_

#include "ili9341.h"
#include <stdio.h>
#include <string.h>

// Set color definitions
#define	WHITE_BLACK   0
#define	BLACK_WHITE   1
#define	WHITE_BLUE    2

void TFT_init(void);
void WindowDraw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t setcolor, const char* str);
void initializeButtons(uint8_t col, uint8_t row, uint8_t h);
void drawButton(uint16_t setcolor, uint8_t b, char *str);
uint8_t contains(uint16_t touch_X, uint16_t touch_Y, uint8_t b);
void checkButtons(uint8_t item);
#endif /* TFT_PROC_H_ */
