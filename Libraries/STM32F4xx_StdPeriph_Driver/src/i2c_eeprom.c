#include "stm32f4xx.h"
#include "i2c_eeprom.h"
#define FAST_I2C_MODE

uint8_t I2c_Buf_Write[200];

/*********************************************************************/
void I2C2_Configuration(void)
{
#ifdef FAST_I2C_MODE
 #define I2C_SPEED 400000
 #define I2C_DUTYCYCLE I2C_DutyCycle_16_9  
#else /* STANDARD_I2C_MODE*/
 #define I2C_SPEED 100000
 #define I2C_DUTYCYCLE I2C_DutyCycle_2
#endif /* FAST_I2C_MODE*/
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	I2C_InitTypeDef   I2C_InitStructure; 
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // enable pull up resistors
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_I2C2);   // only connect to 
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_I2C2);   // only connect to 	
	
/************************************* Master ******************************/
  /* I2C De-initialize */
  I2C_DeInit(I2C2);
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DUTYCYCLE;
  I2C_InitStructure.I2C_OwnAddress1 = 0;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C2, &I2C_InitStructure);
 /* I2C ENABLE */
  I2C_Cmd(I2C2, ENABLE); 
}

/*********************************************************************/
void I2C_ByteWrite(I2C_TypeDef* I2Cx, u8 slaveAdd, u8 regAdd, u8* pBuffer)
{
  /* Send START condition */
  I2C_GenerateSTART(I2Cx, ENABLE);

		/* Test on EV5 and clear it */
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

  /* Send MPU6050 address for write */
  I2C_Send7bitAddress(I2Cx, slaveAdd, I2C_Direction_Transmitter);

		/* Test on EV6 and clear it */
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  /* Send the MPU6050's internal address to write to */
  I2C_SendData(I2Cx, regAdd);

		/* Test on EV8 and clear it */
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  /* Send the byte to be written */
  I2C_SendData(I2Cx, *pBuffer);

		/* Test on EV8 and clear it */
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  /* Send STOP condition */
  I2C_GenerateSTOP(I2Cx, ENABLE);
}

/*********************************************************************/
void I2C_BufferWrite(I2C_TypeDef* I2Cx, u8 slaveAdd, u8 regAdd, u8* pBuffer, u8 NumByteToWrite)
{
		while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY)); 
    
  /* Send START condition */
  I2C_GenerateSTART(I2Cx, ENABLE);
  
		/* Test on EV5 and clear it */
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)); 
  
  /* Send EEPROM address for write */
  I2C_Send7bitAddress(I2Cx, slaveAdd, I2C_Direction_Transmitter);
  
		/* Test on EV6 and clear it */
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));  

  /* Send the EEPROM's internal address to write to */    
  I2C_SendData(I2Cx, regAdd);  

		/* Test on EV8 and clear it */
		while(! I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	  /* Send the EEPROM's internal address to write to */    
  I2C_SendData(I2Cx, regAdd);  



		/* Test on EV8 and clear it */
		while(! I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  /* While there is data to be written */
  while(NumByteToWrite--)  
  {
    /* Send the current byte */
    I2C_SendData(I2Cx, *pBuffer); 

    /* Point to the next byte to be written */
    pBuffer++; 
  
		/* Test on EV8 and clear it */
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  }

  /* Send STOP condition */
  I2C_GenerateSTOP(I2Cx, ENABLE);
}

/*********************************************************************/
void I2C_BufferRead(I2C_TypeDef* I2Cx, u8 slaveAdd, u8 regReadAddr, u8 *pBuffer, u16 NumByteToRead)
{  
/* While the bus is busy */
  while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

  /* Send START condition */
  I2C_GenerateSTART(I2Cx, ENABLE);

  /* Test on EV5 and clear it */
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

  /* Send MPU6050 address for write */
  I2C_Send7bitAddress(I2Cx, slaveAdd, I2C_Direction_Transmitter); 

  /* Test on EV6 and clear it */
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  /* Clear EV6 by setting again the PE bit */
  I2C_Cmd(I2Cx, ENABLE);

  /* Send the MPU6050's internal address to write to */
  I2C_SendData(I2Cx, regReadAddr);

  /* Test on EV8 and clear it */
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(I2Cx, regReadAddr);

  /* Test on EV8 and clear it */
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  /* Send STRAT condition a second time */
  I2C_GenerateSTART(I2Cx, ENABLE);

  /* Test on EV5 and clear it */
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

  /* Send MPU6050 address for read */
  I2C_Send7bitAddress(I2Cx, slaveAdd, I2C_Direction_Receiver);

  /* Test on EV6 and clear it */
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

  /* While there is data to be read */
  while(NumByteToRead)
  {
    if(NumByteToRead == 1)
    {
      /* Disable Acknowledgement */
      I2C_AcknowledgeConfig(I2Cx, DISABLE);

      /* Send STOP Condition */
      I2C_GenerateSTOP(I2Cx, ENABLE);
    }

    /* Test on EV7 and clear it */
    if(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
      /* Read a byte from the MPU6050 */
      *pBuffer = I2C_ReceiveData(I2Cx);

      /* Point to the next location where the byte read will be saved */
      pBuffer++;

      /* Decrement the read bytes counter */
      NumByteToRead--;
    }
  }

  /* Enable Acknowledgement to be ready for another reception */
  I2C_AcknowledgeConfig(I2Cx, ENABLE);
}
