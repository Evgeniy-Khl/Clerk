#ifndef DS18B20_H_
#define DS18B20_H_

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "tft_proc.h"
//#include <stdlib.h>
//#include <stdint.h>

//--------------------------------------------------
#define TUNING      170
#define SKIP_ROM    0
#define NO_SKIP_ROM 1

//--------------------------------------------------
void DelayMicro(__IO uint32_t micros);
uint8_t ds18b20_Reset(void);
void ds18b20_port_init(void);
uint8_t ds18b20_SearhRom(uint8_t *Addr);
uint8_t ds18b20_ReadBit(void);
uint8_t ds18b20_ReadByte(void);
void ds18b20_WriteBit(uint8_t bit);
void ds18b20_WriteByte(uint8_t dt);
uint8_t ds18b20_count(uint8_t amount);
void ds18b20_Convert_T(void);
void ds18b20_ReadStratcpad(uint16_t y_pos, uint8_t DevNum);
void ds18b20_WriteScratchpad(uint8_t DevNum, uint8_t th, int8_t tl);
uint16_t CRC16(uint8_t *pcBlock, uint16_t len);
uint8_t ds18b20_GetSign(uint16_t dt);
float ds18b20_Convert(uint16_t dt);
void displ_status(void);
void displ_T(void);
void temperature_check(void);
uint8_t dallas_crc8(uint8_t * data, uint8_t size);
uint8_t readDHT(void);

#endif /* DS18B20_H_ */
