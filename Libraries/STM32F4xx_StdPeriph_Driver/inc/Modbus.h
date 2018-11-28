#ifndef _Modbus_H_
#define _Modbus_H_
#include "stm32f4xx.h"
#include "Sim800A.h"
#include "math.h"

#define Mas_trans GPIO_SetBits(GPIOD, GPIO_Pin_13)
#define Mas_recei GPIO_ResetBits(GPIOD, GPIO_Pin_13)

#define Sla_trans GPIO_SetBits(GPIOD, GPIO_Pin_14)
#define Sla_recei GPIO_ResetBits(GPIOD, GPIO_Pin_14)


extern volatile unsigned int cntTimeOutFunc;
extern volatile u8 funcWaiting;

extern volatile uint8_t sadd;

extern void Uart_puts(USART_TypeDef *USARTx,volatile char *str);
extern void Uart_putc(USART_TypeDef *USARTx,uint8_t ch);
//extern void Delay(__IO uint32_t nCount);			//No need to extern
extern void split_bit(uint8_t data, uint8_t *dataAr);

void USART1_Configuration(unsigned int BaudRate);
void USART2_Configuration(unsigned int BaudRate);

void Send_uart_data01(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data);
void Send_uart_data02(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data);
void Send_uart_data(USART_TypeDef *USARTx, uint8_t Add, uint8_t Func, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data);
void Send_uart_data15(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, volatile uint8_t *data);
void Send_uart_data16(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t Len1, uint8_t Len2, uint8_t *data);
void Send_uart_data05(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t DA1, uint8_t DA2);
void Send_uart_data06(USART_TypeDef *USARTx, uint8_t Add, uint8_t SA1, uint8_t SA2, uint8_t DA1, uint8_t DA2);
uint16_t modbus_RTU(uint8_t* buf, int len);
uint16_t modbus_RTU_respone(uint8_t* buf, char lenstr, uint16_t Start_address, uint16_t Length);
void resetRegisterSlave(void);
void Rs485_respone3(USART_TypeDef *USARTx, uint16_t Start_address, uint16_t Length);
void Rs485_respone16(USART_TypeDef *USARTx, uint8_t *data);
void waitRespone(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);

#endif   
