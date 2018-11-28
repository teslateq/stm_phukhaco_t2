#ifndef _Uart_H_
#define _Uart_H_

#include "stm32f4xx.h"
#include "math.h"
#include "stdio.h"
void Uart_putc(USART_TypeDef *USARTx,uint8_t ch);
void Uart_puts(USART_TypeDef *USARTx,volatile char *str);

void Uart_putc(USART_TypeDef *USARTx,uint8_t ch)
{
	while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET){}
	USART_SendData(USARTx,ch);
}

void Uart_puts(USART_TypeDef *USARTx,volatile char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
		USART_SendData(USARTx, *str);
		*str++;
	}
}

//void Send_uart_data(USART_TypeDef *USARTx,uint16_t tdata)
//{
//	uint8_t m=0;   //45*1000 + 5*10 + 1 ...45000+50+1 = 45051
//    temp_data[0]=tdata/1000;
//    temp_data[1]=(tdata%1000)/10;
//    temp_data[2]=tdata%10;	
//	//------ send data to Processing via USART3 -------
//	Uart_putc(USART1,255);//start byte
//	for(m=0;m<3;m++)
//	{
//		Uart_putc(USART1,temp_data[m]);
//	}
//	Uart_putc(USART1,254);//stop byte
//	//-------------------------------------------------
//}

//void USART1_IRQHandler(void) //for NVIC_RXNE
//{
//	//-------------------------------------------------Start
//	static int cnt = 0;
//	if( USART_GetITStatus(USART1, USART_IT_RXNE))
//	{
//		char ch = USART1->DR;
//		if(ch != '%')
//		{
//			received_str[cnt++] = ch;
//		}
//		else
//		{
//			received_str[cnt] = '\r';
//			cnt = 0;
//		  Uart_puts(USART1,received_str);
//		}
//	}
//	//-------------------------------------------------End
//}


#endif
