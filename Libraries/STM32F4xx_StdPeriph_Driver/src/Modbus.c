#include <stm32f4xx.h>
#include "Modbus.h"

volatile u8 funcWaiting;


volatile uint8_t sadd;
volatile uint8_t temp_data[100];	//8

char received_data[30];
volatile uint8_t data;

uint16_t Fcrc = 0;
uint8_t FcrcLo = 0;
uint8_t FcrcHi = 0;

volatile uint16_t F1crc = 0;
volatile uint8_t F1crcLo = 0;
volatile uint8_t F1crcHi = 0;

volatile uint16_t crc3i, crc3o;
volatile uint8_t crcLo3i, crcLo3o;
volatile uint8_t crcHi3i, crcHi3o;



//--------------------------------------------------------------USART1

volatile unsigned int cntTimeOutFunc;
volatile uint8_t received_str[50];
volatile int16_t cnt = 0;
volatile uint8_t start = 0;
enum _parseState{
	DETECH_ADD,
	DETECH_FUNC,
	FUNC01,
	FUNC02,
	FUNC0304,
	FUNC0506,
	FUNC15,
	FUNC16,
	CONTENT_FUNC01,
	CONTENT_FUNC02,
	CONTENT_FUNC0304,
	DETECH_LEN,
	DETECH_CONTENT,
	GET_CRC01,
	GET_CRC02,
	GET_CRC,
	GET_CRC0506,
	GET_CRC15,
	GET_CRC16,
	CHECK_CRC
};
volatile uint8_t parseState = DETECH_ADD;


//-------------------------------------------------------------USART2
volatile int8_t start3;
volatile int16_t cnt3;
volatile uint8_t FC3;
volatile uint16_t SA3, L3;
volatile char received_str3[20];	//received command fr modbus out
volatile uint16_t crc3i, crc3o;
volatile uint8_t crcLo3i, crcLo3o;
volatile uint8_t crcHi3i, crcHi3o;
uint16_t modbus_RTU(uint8_t* buf, int len);
void Rs485_respone3(USART_TypeDef *USARTx, uint16_t Start_address, uint16_t Length);
uint16_t modbus_RTU_respone(uint8_t* buf, char lenstr, uint16_t Start_address, uint16_t Length);





void USART1_Configuration(unsigned int BaudRate) {
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//for modbus in
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* Configure USART Tx as alternate function  */ //B6:Tx  B7:Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_USART1); 
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_USART1); 
	
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(USART1, &USART_InitStructure);
  
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
  USART_Cmd(USART1, ENABLE);  
}


void USART2_Configuration(unsigned int BaudRate) {
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART2); 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART2); 
	
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(USART2, &USART_InitStructure);
  
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
  USART_Cmd(USART2, ENABLE);
}
void waitRespone(void) {
	funcWaiting = 1;
	
	int timeOut = 0xffffff;
	while(funcWaiting) {
		timeOut--;
		if(!timeOut) {
			
			if(temp_data[1] == 0x02)
				cntTimeOutFunc++;
			if(cntTimeOutFunc >= 1999999999)
				cntTimeOutFunc = 0;
			
			break;
		}
	}
}
void Send_uart_data01(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data) {
	Mas_trans;
	uint8_t m=0;
	temp_data[0]= Add;
	temp_data[1]= 0x01;
	temp_data[2]= SA1;
	temp_data[3]= SA2;
	temp_data[4]= Len1;
	temp_data[5]= Len2;
	Fcrc = modbus_RTU(temp_data,6);
	FcrcLo = Fcrc;
	FcrcHi = Fcrc>>8;	
	temp_data[6]= FcrcLo;	//CRC Lo
	temp_data[7]= FcrcHi;	//CRC Hi
	
	for(m=0;m<8+1;m++) {
		Uart_putc(USARTx,temp_data[m]);
	}
	Mas_recei;
	
	waitRespone();
	
	for(int i=3; i<received_str[2] + 3; i++)
		data[i-3] = received_str[i];
	
	for(int i=0; i<50; i++)
		received_str[i] = 0;
	parseState = DETECH_ADD;
}
void Send_uart_data02(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data) {
	Mas_trans;
	uint8_t m=0;
	temp_data[0]= Add;
	temp_data[1]= 0x02;
	temp_data[2]= SA1;
	temp_data[3]= SA2;
	temp_data[4]= Len1;
	temp_data[5]= Len2;
	Fcrc = modbus_RTU(temp_data,6);
	FcrcLo = Fcrc;
	FcrcHi = Fcrc>>8;	
	temp_data[6]= FcrcLo;	//CRC Lo
	temp_data[7]= FcrcHi;	//CRC Hi
	
	for(m=0;m<8+1;m++) {
		Uart_putc(USARTx,temp_data[m]);
	}
	Mas_recei;
	
	waitRespone();
	
	for(int i=3; i<received_str[2] + 3; i++)
		data[i-3] = received_str[i];
	
	for(int i=0; i<50; i++)
		received_str[i] = 0;
	parseState = DETECH_ADD;
}

void Send_uart_data(USART_TypeDef *USARTx, uint8_t Add, uint8_t Func, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data) {	
//01 03 00 00 00 12 C5 C7
//	01 03 00 00 00 02 C4 0B -Read 2 byte fr addr 0x00
//	01 03 00 00 00 07 04 08 -Read 7 byte fr addr 0x00
//	01 03 00 00 00 14 45 C5 -Read 20 byte fr addr 0x00
//	01 03 00 0F 00 0B 34 0E -Read 11 byte fr addr 15(0x0f)
//	01 03 00 1c 00 02	05 CD -Read 2 byte fr addr 28(0x1c)
//  01 03 00 01 00 01 D5 CA -			1 byte fr addr 0x01
	Mas_trans;
	uint8_t m=0;   
	temp_data[0]= Add;
	temp_data[1]= Func;
	temp_data[2]= SA1;
	temp_data[3]= SA2;
	temp_data[4]= Len1;
	temp_data[5]= Len2;
	Fcrc = modbus_RTU(temp_data,6);
	FcrcLo = Fcrc; //0xca25&0xff    1100 1010 0010 0101 & 1111 1111 = 0025
	FcrcHi = Fcrc>>8;
	temp_data[6]= FcrcLo;	//CRC Lo
	temp_data[7]= FcrcHi;	//CRC Hi
	
	for(m=0;m<8+1;m++) {
		Uart_putc(USARTx,temp_data[m]);
	}
	Mas_recei;
	
	waitRespone();
	
	
	for(int i=3; i<temp_data[5]*2+3; i++)
		data[i-3] = received_str[i];

	
	for(int i=0; i<50; i++)
		received_str[i] = 0;
	parseState = DETECH_ADD;
	//-------------------------------------------------
}

void Send_uart_data15(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, volatile uint8_t *data)  {
//RQ: 01 10 01 0B 00 01 02 00 01 76 2B 
	//RP: 01 10 01 0B 00 01 71 F7 
	uint16_t Len = Len1<<8 | Len2;
	uint8_t Len_byte = ceil( (double)Len/8 );
	
	Mas_trans;
	uint8_t m=0;   
	temp_data[0] = Add;
	temp_data[1] = 0x0f;
	temp_data[2] = SA1;
	temp_data[3] = SA2;
	temp_data[4] = Len1;
	temp_data[5] = Len2;
	temp_data[6] = Len_byte;
		
	for(int i=0; i<temp_data[6]; i++){
		temp_data[i+7] = data[i];
	}
	Fcrc = modbus_RTU(temp_data,temp_data[6]+7);
	FcrcLo = Fcrc; //0xca25&0xff    1100 1010 0010 0101 & 1111 1111 = 0025
	FcrcHi = Fcrc>>8;
	temp_data[temp_data[6]+7]= FcrcLo;	//CRC Lo
	temp_data[temp_data[6]+8]= FcrcHi;	//CRC Hi
	
	for(m=0; m<temp_data[6]+9+1; m++)
	{
		Uart_putc(USARTx,temp_data[m]);
	}
	Mas_recei;
	
	waitRespone();
	
	for(int i=0; i<50; i++)
		received_str[i] = 0;
	parseState = DETECH_ADD;
	//-------------------------------------------------
}

void Send_uart_data16(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data) {
	//RQ: 01 10 01 0B 00 01 02 00 01 76 2B 
	//RP: 01 10 01 0B 00 01 71 F7 
	Mas_trans;
	uint8_t numByte = (Len1<<8 | Len2) * 2;	
	uint8_t m=0;   
	temp_data[0]= Add;
	temp_data[1]= 0x10;
	temp_data[2]= SA1;
	temp_data[3]= SA2;
	temp_data[4]= Len1;
	temp_data[5]= Len2;
	temp_data[6]= numByte;
	for(int i=0; i<numByte; i++)	//7 8 9 10
		temp_data[i+7] = data[i];
	
	Fcrc = modbus_RTU(temp_data, 7+numByte);
	FcrcLo = Fcrc;
	FcrcHi = Fcrc>>8;
	temp_data[7+numByte]= FcrcLo;
	temp_data[8+numByte]= FcrcHi;	
	
	for(m=0; m<9+numByte+1; m++)
		Uart_putc(USARTx,temp_data[m]);
	
	Mas_recei;
	
	waitRespone();
	
	for(int i=0; i<50; i++)
		received_str[i] = 0;
	parseState = DETECH_ADD;
	//-------------------------------------------------
}
void Send_uart_data05(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t DA1, uint8_t DA2) {	
	Mas_trans;   
	
	uint16_t crc5;
	uint8_t m=0, crc5Lo, crc5Hi;
	temp_data[0]= Add;
	temp_data[1]= 0x05;
	temp_data[2]= SA1;
	temp_data[3]= SA2;
	temp_data[4]= DA1;
	temp_data[5]= DA2;
	crc5 = modbus_RTU(temp_data, 6);
	crc5Lo = crc5;
	crc5Hi = crc5 >> 8;
	temp_data[6] = crc5Lo;
	temp_data[7] = crc5Hi;
	for(m=0;m<8+1;m++)
	{
		Uart_putc(USARTx,temp_data[m]);
	}
	
	Mas_recei;
	
	waitRespone();
	
	for(int i=0; i<50; i++)
		received_str[i] = 0;
	parseState = DETECH_ADD;
}
void Send_uart_data06(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t DA1, uint8_t DA2) {
	Mas_trans;   
	
	uint16_t crc5;
	uint8_t m=0, crc5Lo, crc5Hi;
	temp_data[0]= Add;
	temp_data[1]= 0x06;
	temp_data[2]= SA1;
	temp_data[3]= SA2;
	temp_data[4]= DA1;
	temp_data[5]= DA2;
	crc5 = modbus_RTU(temp_data, 6);
	crc5Lo = crc5;
	crc5Hi = crc5 >> 8;
	temp_data[6] = crc5Lo;
	temp_data[7] = crc5Hi;
	for(m=0;m<8+1;m++)
	{
		Uart_putc(USARTx,temp_data[m]);
	}
	
	Mas_recei;
	
	waitRespone();
	
	for(int i=0; i<50; i++)
		received_str[i] = 0;
	parseState = DETECH_ADD;
}
uint16_t modbus_RTU(uint8_t* buf, int len) {
	uint16_t crc = 0xFFFF;

	for (int pos = 0; pos < len; pos++) {
		crc ^= (uint16_t)buf[pos]; 								// XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {					// Loop over each bit 8 7 6 5 4 3 2 1
			if ((crc & 0x0001) != 0) { 						// If the LSB is set
 				crc >>= 1;
				crc ^= 0xA001;											// Shift right and XOR 0xA001	
			}
			else
				crc >>= 1;
		}					
	}
	return crc;
}

uint16_t modbus_RTU_respone(uint8_t* buf, char lenstr, uint16_t Start_address, uint16_t Length) {
	uint16_t crc = 0xFFFF;
	
	crc ^= (uint16_t)sadd;
	for (int i = 8; i != 0; i--) {					// Loop over each bit 8 7 6 5 4 3 2 1
		if ((crc & 0x0001) != 0) { 						// If the LSB is set
 			crc >>= 1;
			crc ^= 0xA001;											// Shift right and XOR 0xA001	
		}
		else
			crc >>= 1;
	}
	
	crc ^= (uint16_t)0x03;
	for (int i = 8; i != 0; i--) {					// Loop over each bit 8 7 6 5 4 3 2 1
		if ((crc & 0x0001) != 0) { 						// If the LSB is set
 			crc >>= 1;
			crc ^= 0xA001;											// Shift right and XOR 0xA001	
		}
		else
			crc >>= 1;
	}
	
	crc ^= (uint16_t)(lenstr*2);
	for (int i = 8; i != 0; i--) {					// Loop over each bit 8 7 6 5 4 3 2 1
		if ((crc & 0x0001) != 0) { 						// If the LSB is set
 			crc >>= 1;
			crc ^= 0xA001;											// Shift right and XOR 0xA001	
		}
		else
			crc >>= 1;
	}
	
	for (int pos = Start_address; pos < Start_address+Length*2; pos++) {
		crc ^= (uint16_t)buf[pos]; 								// XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {					// Loop over each bit 8 7 6 5 4 3 2 1
			if ((crc & 0x0001) != 0) { 						// If the LSB is set
 				crc >>= 1;
				crc ^= 0xA001;											// Shift right and XOR 0xA001	
			}
			else
				crc >>= 1;
		}					
	}
	return crc;
}

void resetRegisterSlave(void) {
		for(int i=0; i<20; i++)		
			received_str3[i] = 0;
		FC3 = 0; SA3 = 0; L3 = 0;
		start3 = 0;	
}

void Rs485_respone3(USART_TypeDef *USARTx, uint16_t Start_address, uint16_t Length) {
	Sla_trans;
	crc3o = modbus_RTU_respone(received_data, received_str3[5], Start_address, Length);
	crcLo3o = crc3o;
	crcHi3o = crc3o>>8;
	Uart_putc(USARTx, sadd);
	Uart_putc(USARTx, 0x03);
	Uart_putc(USARTx, received_str3[5]*2);
	for(int i = Start_address; i < Start_address+Length*2; i++)
		Uart_putc(USARTx, received_data[i]);	//data Usart1
	Uart_putc(USARTx, crcLo3o);
	Uart_putc(USARTx, crcHi3o);
	
	Uart_putc(USARTx, 0);
	Sla_recei;
}

void Rs485_respone16(USART_TypeDef *USARTx, uint8_t *data) {
	Sla_trans;
	crc3o = modbus_RTU(data, 6);
	crcLo3o = crc3o;
	crcHi3o = crc3o >> 8;
	for(int i = 0; i < 6; i++)
		Uart_putc(USARTx, data[i]);
	Uart_putc(USARTx,crcLo3o);
	Uart_putc(USARTx, crcHi3o);
	
	Uart_putc(USARTx, 0);
	Sla_recei;
}


void split_bit(uint8_t data, uint8_t *dataAr) {
	uint8_t temp = data;
	for(uint8_t i=0; i<8; i++) {
		dataAr[i] = temp & 0x01;
		temp = temp >> 1;
	}
}

void USART1_IRQHandler(void) //for NVIC_RXNE
{
	//modbus_master
	//-------------------------------------------------Start
	if( USART_GetITStatus(USART1, USART_IT_RXNE))
	{
		char ch = USART1->DR;
		
		switch(parseState) {
			case DETECH_ADD:
			{
				if(ch == temp_data[0]) {
					received_str[0] = temp_data[0];
					parseState = DETECH_FUNC;
				}
				break;
			}
			case DETECH_FUNC:
			{
				if(ch == temp_data[1]) {
					received_str[1] = temp_data[1];
					if(ch == 0x03 || ch == 0x04)
						parseState = FUNC0304;
					else if(ch == 0x05 || ch == 0x06)
						parseState = FUNC0506;
					else if(ch == 0x0F)
						parseState = FUNC15;
					else if(ch == 0x10)
						parseState = FUNC16;
					else if(ch == 0x02)
						parseState = FUNC02;
					else if(ch == 0x01)
						parseState = FUNC01;
					else 
						parseState = DETECH_ADD;
				}
				else
					parseState = DETECH_ADD;
				break;
			}
			case FUNC0304:
			{
				if(ch == temp_data[5]*2)
				{
					received_str[2] = temp_data[5]*2;
					cnt = 2;
					parseState = CONTENT_FUNC0304;
				}
				break;
			}
			case CONTENT_FUNC0304:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= temp_data[5]*2 + 2)	//4 + 2 = 6
					parseState = GET_CRC;
				break;
			}
			
			case GET_CRC:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= temp_data[5]*2 + 4)	//4 + 4 = 8				01 03 00 00 00 02 crc crc -> 01 03 04 0a 0a 0a 0a crc crc
				{
					F1crc = modbus_RTU(received_str,temp_data[5]*2 + 3);
					F1crcLo = F1crc;
					F1crcHi = F1crc>>8;
					
					if(received_str[temp_data[5]*2+3] == F1crcLo && received_str[temp_data[5]*2+4] == F1crcHi)
						funcWaiting = 0;
					
					cnt = 0;	
					parseState = DETECH_ADD;
				}
				break;
			}
			
			case FUNC0506:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= 5)
					parseState = GET_CRC15;
				break;
			}
			
			case GET_CRC0506:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= 7)
				{
					F1crc = modbus_RTU(received_str, 6);
					F1crcLo = F1crc;
					F1crcHi = F1crc>>8;
					
					if(received_str[6] == F1crcLo && received_str[7] == F1crcHi)
						funcWaiting = 0;
					
					cnt = 0;
					parseState = DETECH_ADD;
				}
				break;
			}
			
			case FUNC15:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= 5)
					parseState = GET_CRC15;
				break;
			}
			
			case GET_CRC15:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= 7)
				{
					F1crc = modbus_RTU(received_str, 6);
					F1crcLo = F1crc;
					F1crcHi = F1crc>>8;
					
					if(received_str[6] == F1crcLo && received_str[7] == F1crcHi)
						funcWaiting = 0;
					
					cnt = 0;
					parseState = DETECH_ADD;
				}
				break;
			}
			case FUNC16:
			{	//0 [1] 2 3 4 5
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= 5)
					parseState = GET_CRC16;
				break;
			}
			case GET_CRC16:
			{	//0 1  2 3 4 [5]  6 7
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= 7)
				{
					F1crc = modbus_RTU(received_str, 6);
					F1crcLo = F1crc;
					F1crcHi = F1crc>>8;
					
					if(received_str[6] == F1crcLo && received_str[7] == F1crcHi)
						funcWaiting = 0;
					
					cnt = 0;
					parseState = DETECH_ADD;
				}
				break;
			}
			
			case FUNC01:
			{
				received_str[2] = ch;
				cnt = 2;											//for the next step
				parseState = CONTENT_FUNC01;
				break;
			}
			case CONTENT_FUNC01:
			{
				cnt++;
				received_str[cnt] = ch;			// 3 < 6	.. 3 4 5					
				if(cnt >= 2 + received_str[2])
					parseState = GET_CRC01;
				break;
			}
			case GET_CRC01:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= received_str[2] + 4)	//
				{
					F1crc = modbus_RTU(received_str, received_str[2] + 3);
					F1crcLo = F1crc;
					F1crcHi = F1crc>>8;
					
					if(received_str[received_str[2]+3] == F1crcLo && received_str[received_str[2]+4] == F1crcHi)
						funcWaiting = 0;
					
					cnt = 0;
					parseState = DETECH_ADD;
				}
				break;
			}
			
			case FUNC02:
			{
				received_str[2] = ch;
				cnt = 2;											//for the next step
				parseState = CONTENT_FUNC02;
				break;
			}
			case CONTENT_FUNC02:
			{
				cnt++;
				received_str[cnt] = ch;			// 3 < 6	.. 3 4 5					
				if(cnt >= 2 + received_str[2])
					parseState = GET_CRC02;
				break;
			}
			case GET_CRC02:
			{
				cnt++;
				received_str[cnt] = ch;
				if(cnt >= received_str[2] + 4)	//
				{
					F1crc = modbus_RTU(received_str, received_str[2] + 3);
					F1crcLo = F1crc;
					F1crcHi = F1crc>>8;
					
					if(received_str[received_str[2]+3] == F1crcLo && received_str[received_str[2]+4] == F1crcHi)
						funcWaiting = 0;
					
					cnt = 0;
					parseState = DETECH_ADD;
				}
				break;
			}
			
			
			
			default:
				break;
		}
		
		USART_ClearFlag(USART1,USART_IT_RXNE);
	}
	//-------------------------------------------------End
}

void USART2_IRQHandler(void) //for NVIC_RXNE
{//modbus slave
	
	//-------------------------------------------------Start
	if( USART_GetITStatus(USART2, USART_IT_RXNE))
	{
		char ch3 = USART2->DR;
		
		if(start3 == 0) {
			if(ch3 == sadd)	//Cause slave, should check the address first
			{
				cnt3 = 0;
				start3 = 1;
			}
		}
		if(start3 == 1) {
			if(cnt3 < 6) {
				received_str3[cnt3] = ch3;	// 0 1 2 3 4 5 6 7 
				cnt3++;
			}
			else if(cnt3 >= 6)
			{
				
				FC3 = (uint8_t)received_str3[1];
				SA3 = (received_str3[2]<<8)|received_str3[3];
				L3 = (received_str3[4]<<8)|received_str3[5];
				
				switch(FC3)
				{
					case 3:
						received_str3[cnt3] = ch3;
						cnt3++;
						if(cnt3 >= 8)
						{
							received_data[0] = 1;
							received_data[1] = 2;
							received_data[2] = 3;
							crc3i = modbus_RTU(received_str3,6);
							crcLo3i = crc3i;
							crcHi3i = crc3i>>8;
							if(crcLo3i == received_str3[6] && crcHi3i == received_str3[7])
							{
								if(SA3+L3*2>=0 && SA3+L3*2<=30) {			//received_data[30]
									Rs485_respone3(USART2, SA3, L3);
								}
							}
							resetRegisterSlave();
						}
						break;
						
					case 16:
						received_str3[cnt3] = ch3;
						cnt3++;
						if(cnt3 >= 9+L3*2)	//11
						{
							crc3i = modbus_RTU(received_str3, 7+L3*2);	//9
							crcLo3i = crc3i;
							crcHi3i = crc3i >> 8;
							if(crcLo3i == received_str3[7+L3*2] && crcHi3i == received_str3[8+L3*2])
							{
								Rs485_respone16(USART2, received_str3);
							}
							resetRegisterSlave();
						}
						
						break;
						
					default: break;
				}
				
			}	
			
		}
		USART_ClearFlag(USART2, USART_IT_RXNE);
	}
	//-------------------------------------------------End
}
