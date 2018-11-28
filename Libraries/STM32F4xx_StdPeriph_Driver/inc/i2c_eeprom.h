#ifndef __i2c_eeprom_H
#define __i2c_eeprom_H
#include "stm32f4xx.h"
#include "basicFunc.h"

#define EEPROM_ADD_24lc512 0xA0
#define EE_Write	0xA0
#define EE_Read		0xA1

extern uint8_t I2c_Buf_Write[200];

void I2C2_Configuration(void);
void I2C_ByteWrite(I2C_TypeDef* I2Cx, u8 slaveAdd, u8 regAdd, u8* pBuffer);
void I2C_BufferWrite(I2C_TypeDef* I2Cx, u8 slaveAdd, u8 regAdd, u8* pBuffer, u8 NumByteToWrite);
void I2C_BufferRead(I2C_TypeDef* I2Cx, u8 slaveAdd, u8 regReadAddr, u8 *pBuffer, u16 NumByteToRead);




#endif


