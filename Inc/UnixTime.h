/***************************************************************************
 * Ця бібліотека призначена для роботи з RTC STM32F1xx за допомоги UnixTime*
 * Дозволяє перетворювати з лічильника в колодар (день, місяц, рік)		   *
 * і час (години, хвилини, секунди), і навпаки							   *
 ***************************************************************************/
#include "main.h"
#define SECOND_A_DAY 86400

uint32_t colodarSetting(int year, char month, char day, char hour, char min, char sec);
//void counterToColodar (unsigned long counter, unixColodar * unixTime);
unsigned long colodarToCounter (void);
