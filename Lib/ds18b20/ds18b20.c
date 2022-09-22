#include "ds18b20.h"

//--------------------------------------------------
uint8_t LastDeviceFlag;
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t ROM_NO[8];
extern char buffTFT[];
extern uint8_t familycode[8][8];
extern uint8_t ds18b20_amount;
extern int16_t ds18b20_val[], pvT, pvRH;

//--------------------------------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
micros *= (SystemCoreClock / 1000000) / 8;
/* Wait till done */
while (micros--) ;
}
//--------------------------------------------------
void ds18b20_port_init(void){
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
  GPIOB->CRH |= GPIO_CRH_MODE11;
  GPIOB->CRH |= GPIO_CRH_CNF11_0;
  GPIOB->CRH &= ~GPIO_CRH_CNF11_1;
}
//-----------------------------------------------
uint8_t ds18b20_SearhRom(uint8_t *Addr){
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;
  //проинициализируем переменные
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;
	if (!LastDeviceFlag){
		ds18b20_Reset();
		ds18b20_WriteByte(0xF0);
	}
	do{
		id_bit = ds18b20_ReadBit();
		cmp_id_bit = ds18b20_ReadBit();
		if ((id_bit == 1) && (cmp_id_bit == 1))	break;
		else{
			if (id_bit != cmp_id_bit)
				search_direction = id_bit; // bit write value for search
			else{
				if (id_bit_number < LastDiscrepancy)
					search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
				else
					search_direction = (id_bit_number == LastDiscrepancy);
				if (search_direction == 0){
					last_zero = id_bit_number;
					if (last_zero < 9) LastFamilyDiscrepancy = last_zero;
				}
			}
			if (search_direction == 1) ROM_NO[rom_byte_number] |= rom_byte_mask;
			else ROM_NO[rom_byte_number] &= ~rom_byte_mask;
			ds18b20_WriteBit(search_direction);
			id_bit_number++;
			rom_byte_mask <<= 1;
			if (rom_byte_mask == 0){
				rom_byte_number++;
				rom_byte_mask = 1;
			}
		}
  } while(rom_byte_number < 8); // считываем байты с 0 до 7 в цикле
	if (!(id_bit_number < 65)){
	  // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
		LastDiscrepancy = last_zero;
		// check for last device
		if (LastDiscrepancy == 0)	LastDeviceFlag = 1;
		search_result = 1;	
  }
	if (!search_result || !ROM_NO[0]){
		LastDiscrepancy = 0;
		LastDeviceFlag = 0;
		LastFamilyDiscrepancy = 0;
		search_result = 0;
	}
	else{
    for (int i = 0; i < 8; i++) Addr[i] = ROM_NO[i];
  }	
  return search_result;
}
//-----------------------------------------------
uint8_t ds18b20_Reset(void){
  uint8_t status;
  GPIOB->BSRR =GPIO_BSRR_BR11;//низкий уровень (Сбросили 11 бит порта B )
  //GPIOB->ODR &= ~GPIO_ODR_ODR11;//низкий уровень
  DelayMicro(485);//задержка как минимум на 480 микросекунд
  GPIOB->BSRR =GPIO_BSRR_BS11;//высокий уровень (Установили 11 бит порта B )
  //GPIOB->ODR |= GPIO_ODR_ODR11;//высокий уровень
  DelayMicro(65);//задержка как минимум на 60 микросекунд
  status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
  DelayMicro(500);//задержка как минимум на 480 микросекунд
  //(на всякий случай подождём побольше, так как могут быть неточности в задержке)
  return (status ? 1 : 0);//вернём результат
}
//--------------------------------------------------
uint8_t ds18b20_ReadBit(void){
  uint8_t bit = 0;
  GPIOB->BSRR =GPIO_BSRR_BR11;//низкий уровень
  DelayMicro(2);
  GPIOB->BSRR =GPIO_BSRR_BS11;//высокий уровень
  DelayMicro(13);
  bit = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
  DelayMicro(45);
  return bit;
}
//-----------------------------------------------
uint8_t ds18b20_ReadByte(void){
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++)
  data += ds18b20_ReadBit() << i;
  return data;
}
//-----------------------------------------------
void ds18b20_WriteBit(uint8_t bit){
  GPIOB->BSRR =GPIO_BSRR_BR11;//низкий уровень
  DelayMicro(bit ? 3 : 65);
  GPIOB->BSRR =GPIO_BSRR_BS11;//высокий уровень
  DelayMicro(bit ? 65 : 3);
}
//-----------------------------------------------
void ds18b20_WriteByte(uint8_t dt){
  for (uint8_t i = 0; i < 8; i++){
    ds18b20_WriteBit(dt >> i & 1);
    //Delay Protection
    DelayMicro(5);
  }
}
//-----------------------------------------------
uint8_t ds18b20_count(uint8_t amount){
  uint8_t i, dt[8];
  ds18b20_amount = 0;
  for(i = 0; i < amount; i++){
    if(ds18b20_SearhRom(dt)){
      memcpy(familycode[ds18b20_amount],dt,sizeof(dt));
      ds18b20_amount++;
    }
    else break;
  }
  return 0;
}
//-----------------------------------------------
void ds18b20_Convert_T(){
  ds18b20_Reset();
  ds18b20_WriteByte(0xCC);  //SKIP ROM
  ds18b20_WriteByte(0x44);  //CONVERT T
}
//----------------------------------------------------------
void ds18b20_ReadStratcpad(uint16_t y_pos, uint8_t DevNum){
  uint8_t i, crc, dt[8];
  ds18b20_Reset();            // 1 Wire Bus initialization
	ds18b20_WriteByte(0x55);  // Load MATCH ROM [55H] comand
	for(i = 0; i < 8; i++){ds18b20_WriteByte(familycode[DevNum][i]);}
  ds18b20_WriteByte(0xBE);  //READ SCRATCHPAD
  for(i=0;i<8;i++){dt[i] = ds18b20_ReadByte();}
  crc = ds18b20_ReadByte(); // Read CRC byte
  i = dallas_crc8(dt, 8);
  if (i==crc){
    sprintf(buffTFT,"PAD %02X %02X %02X %02X %02X %02X %02X %02X",
       dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7]);
    ILI9341_WriteString(5, y_pos, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
  }
}
//----------------------------------------------------------
void ds18b20_WriteScratchpad(uint8_t DevNum, uint8_t th, int8_t tl)   // ЗАПИСЬ в configuration register
{
 uint8_t i;
  ds18b20_Reset();          // 1 Wire Bus initialization
  ds18b20_WriteByte(0x55);  // Load MATCH ROM [55H] comand
  for(i = 0; i < 8; i++){
			ds18b20_WriteByte(familycode[DevNum][i]);
		}
  ds18b20_WriteByte(0x4E);  // Load WRITE SCRATCHPAD [4EH] command
  ds18b20_WriteByte(th);    // TH
  ds18b20_WriteByte(tl);    // TL
  ds18b20_WriteByte(0x7F);  // configuration register
  ds18b20_Reset();          // 1 Wire Bus initialization
  ds18b20_WriteByte(0xCC);  // Load Skip ROM [CCH] command
  ds18b20_WriteByte(0x48);  // Load COPY SCRATCHPAD [48h] command
}
//----------------------------------------------------------
uint8_t dallas_crc8(uint8_t * data, uint8_t size){
 uint8_t crc = 0;
  for (uint8_t i = 0; i < size; ++i){
    uint8_t inbyte = data[i];
    for (uint8_t j = 0; j < 8; ++j){
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if ( mix ) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}
//----------------------------------------------------------
/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
uint16_t CRC16(uint8_t *pcBlock, uint16_t len){
  uint16_t crc = 0xFFFF;
  uint8_t i;
  while (len--){
    crc ^= *pcBlock++ << 8;
    for (i = 0; i < 8; i++)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  return crc;
}
//----------------------------------------------------------
uint8_t ds18b20_GetSign(uint16_t dt){
  //Проверим 11-й бит
  if (dt&(1<<11)) return 1;
  else return 0;
}
//----------------------------------------------------------
float ds18b20_Convert(uint16_t dt){
  float t;
  t = (float) ((dt&0x07FF)>>4); //отборосим знаковые и дробные биты
  //Прибавим дробную часть
  t += (float)(dt&0x000F) / 16.0f;
  return t;
}

//----------------------------------------------------------
void temperature_check(){
  uint8_t item, byte, crc, try_cnt=0;
  union ds {uint8_t data[8]; int16_t val;} buffer;
  for (item=0; item < ds18b20_amount;){
    ds18b20_Reset(); // 1 Wire Bus initialization
    ds18b20_WriteByte(0x55); // Load MATCH ROM [55H] comand
    for (byte=0; byte < 8; byte++) ds18b20_WriteByte(familycode[item][byte]); // Load cont. byte
    ds18b20_WriteByte(0xBE); // Read Scratchpad command [BE]
    for (byte=0; byte < 8; byte++){
        buffer.data[byte] = ds18b20_ReadByte(); // Read cont. byt
    }
    crc = ds18b20_ReadByte(); // Read CRC byte
    byte = dallas_crc8(buffer.data, 8);
    if (byte==crc){
      try_cnt = 0;
      if (buffer.val<0){
        buffer.val = -buffer.val;
        ds18b20_val[item] = buffer.val*10/16;
        ds18b20_val[item] = -ds18b20_val[0];
      }
      else ds18b20_val[item] =  buffer.val*10/16;
      crc = buffer.data[2]&TUNING;
      if (crc==TUNING){int8_t correction=buffer.data[3]; ds18b20_val[item] +=correction;}// корекция показаний датчика
    }
    else if (++try_cnt > 2) {try_cnt = 0; ds18b20_val[item] = 1990;};// (199) если ошибка более X раз то больше не опрашиваем 
    if (try_cnt==0) item++;
   }
  ds18b20_Convert_T();
}
//------- DHT-21 / DHT-11 ------------------------------------------
uint8_t startDHT(void){
 uint8_t flag=0, status;
  GPIOB->BSRR =GPIO_BSRR_BR11;//низкий уровень (Сбросили 11 бит порта B )
//#if DHT21==0
//  HAL_Delay(30); // MCU Sends out Start Signal to DHT and pull down voltage for at least 18ms to let DHT11 detect the signal.
//#else
  HAL_Delay(10); // MCU Sends out Start Signal to DHT and pull down voltage for at least 18ms to let DHT21 detect the signal.
//#endif
  GPIOB->BSRR =GPIO_BSRR_BS11;//высокий уровень (Установили 11 бит порта B )
  DelayMicro(60);// wait for DHT respond 20-40uS
  status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
   if(!status)
   {
      while(!status){ // low-voltage-level response signal & keeps it for 80us (flag=32 Response to low time finished)
        flag++;
        status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
      }
      if(flag<10) return 0;
      else {          // hi-voltage-level response signal & keeps it for 80us (flag=55 Response to high time finished)
        flag=0;
        while(status){
          flag++;
          status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
        }
      }
      if(flag<10) return 0;
      else return 1;
   }
   else return 0;
}

uint8_t readDHT(void){
 uint8_t i, j, flag=0, status, tem[5];
 static uint8_t err;
 if(startDHT()){
    for(i=0; i<5; i++){
       tem[i]=0;
       for(j=0; j<8; j++){
          tem[i]<<= 1;
//          #asm("cli")     // Global enable interrupts
//        delay_us(30);   // When DHT is sending data to MCU, every bit of data begins with the 50us low-voltage-level
                          // and the length of the following high-voltage-level signal determines whether data bit is "0" or "1"         
          status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
          while(!status){   // ожидаем фронт сигнала
            flag++;
            status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
          }
          DelayMicro(32);   //26-28u voltage-length means data "0" / 70u voltage-length means data "1"
          flag=0;
          status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
          while(status){// ожидаем спад сигнала
            flag++;
            status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
          }
          if(flag>10) tem[i]|= 1;// data "1"
//          #asm("sei")     // Global enable interrupts
        };
     };
    flag=tem[0]+tem[1]+tem[2]+tem[3];
    if(flag==tem[4])
     {
//  #if DHT21==0
//      pvRH=(int)tem[0]*10; pvT[2]=(int)tem[2]*10;
//  #else
      pvRH =(int)tem[0]*256+tem[1]; pvT =(int)tem[2]*256+tem[3];
//  #endif
//     pvRH +=sp[0].spRH;                  // коррекция датчика влажности
      err = 0;
      if(pvRH>1000) pvRH=1000; else if (pvRH<0) pvRH=0;
      return 1;
     }
    else if(++err>3) return 0;  // НЕ верная CRC датчика влажности  errors |=0x10;
  }
 return flag;     // потерян датчик влажности
}
