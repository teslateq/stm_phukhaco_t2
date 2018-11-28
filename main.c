#include "stm32f4xx.h"
#include <stdio.h>
#include <Uart.h>
#include "stdlib.h"
#include "string.h"
#include "basicFunc.h"
#include "Sim800A.h"
#include "Modbus.h"
#include "rtc_internal.h"
#include "adc.h"
#include "i2c_eeprom.h"

USART_InitTypeDef USART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

#define numLoad 50
#define State	1
#define num_ena_sdt 6			// pin

void init_value(void);
int JoinValueModbus(volatile uint8_t *data, uint8_t RsAB);
void push_data2web(USART_TypeDef *USARTx);
void read_temp(void);
void read_volt(void);
void read_write_adam8024(void);
void Send_byte2str(uint8_t data);
void process(void);
void daily_rst(void);
void cne_rst(void);
void del_all_mess(void);																											// pin
void sms_process(void);
void sms_warning(void);
uint8_t check_phone(volatile char *data1, volatile char *str, uint16_t num);

uint8_t Pump1, Pump2, Trash, Warn, WL1, WL2, WL3, WL4, WLState;
uint8_t dataIO_2[10], input_485_2[10], Pump3;

uint16_t senClo;
uint8_t senDataClo[4];

uint8_t temp[2], volt[2];

int cntAlarm;
int rst;
int rstIWDG;

uint8_t oldRead;
int cntEqual;
int cntNEqual;
int cntSum;
uint8_t bias;
uint16_t loadBias;

float tempVxl;
uint8_t t1[30],t2[30],t3[30],t4[30];

// pin
float src;
uint8_t rstChange[5];
char sdt[20][50] = { {"0902555793"}, {"0767543008"}, {"0767543008"}, {"0767543008"}, {"0767543008"}, {"0767543008"} };
uint8_t passCheck, passCheck1;
uint8_t enaSms[5];

uint8_t ADAM8024_out[10], ADAM8024_in[10];
uint8_t DO[10];

int main(void)
{
	SysTick_Config(SystemCoreClock / 1000);
  USART1_Configuration(9600);		//rs485 master
	USART6_Configuration(115200);		//sim
	USART2_Configuration(9600);			//rs485 slave
	ADC_Config();
	I2C2_Configuration();
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	IWDG_SetReload(4090);						//PR: 128 -> 4ms .. RL: 3750 -> 15s
  IWDG_ReloadCounter();
  IWDG_Enable();
	
	
	
	// Xong xoa doan nay
//	for(int i=2; i<2+num_ena_sdt; i++) {
//		I2C_BufferWrite(I2C2, EE_Write, i, (unsigned char*)sdt[i-2], 10);
//		Delay(10);
//	}
//	rstChange[0] = 1;
//	enaSms[0] = 1;
//	I2C_BufferWrite(I2C2, EE_Write, 0x00, rstChange, 1);			Delay(10);
//	I2C_BufferWrite(I2C2, EE_Write, 0x01, enaSms, 1);					Delay(10);
//	Delay(500);
	
	
	
	
	check_statusM95();
	Delay(20000);
	
	init_value();
	
	I2C_BufferRead(I2C2, EE_Write, 0x00, rstChange, 1);
	I2C_BufferRead(I2C2, EE_Write, 0x01, enaSms, 1);
	
	for(int i=2; i<2+num_ena_sdt; i++)
		I2C_BufferRead(I2C2, EE_Write, i, (unsigned char*)sdt[i-2], 10);
	
	GSMD("AT+CMGF=1\n",50);

	IWDG_ReloadCounter();
	while (1)
  {		

		
		// Test init value & data received
		for(int i=0; i<10; i++) {
			t1[i] = sdt[0][i];
			t2[i] = sdt[1][i];
			t3[i] = sdt[2][i];
		}
		t4[0] = web[1][0];
		t4[1] = web[1][1];
		t4[2] = web[1][2];
		t4[3] = web[1][3];
		
		
		daily_rst();
		cne_rst();
		src = adc[1]*51.0/4095.0;
		
		
		
		GSMD("AT+CMGR=1\n",200);
		sms_process();
		
		if(src >= 6 && rstChange[0] == 0) {
			rstChange[0] = 1;
			enaSms[0] = 1;
			I2C_BufferWrite(I2C2, EE_Write, 0x00, rstChange, 1);			Delay(10);
			I2C_BufferWrite(I2C2, EE_Write, 0x01, enaSms, 1);					Delay(10);
			NVIC_SystemReset();
		}
		else if(src < 6) {
			
			if(rstChange[0] == 1) {
					rstChange[0] = 0;
					I2C_BufferWrite(I2C2, EE_Write, 0x00, rstChange, 1);
					Delay(20);
					NVIC_SystemReset();
			}
			if(rstChange[0] == 0) {
				
					if(enaSms[0] == 1) {
						sms_warning();
						
						enaSms[0] = 0;
						I2C_BufferWrite(I2C2, EE_Write, 0x01, enaSms, 1);
					}
			}
			
		}
		
		
		
		if(stepConn == 0)
			check_statusM95_main();
		else if(stepConn == 1)
			check_signal();
		else if(stepConn == 2)
			check_gprs();
		else if(stepConn == 3)
			push_data2web(USART6);
		

		
		IWDG_ReloadCounter();
		while(rstIWDG == 1);
	}
}



int JoinValueModbus(volatile uint8_t *data, uint8_t RsAB) {
	int databack;
	
	if(RsAB == 0)
		databack = data[0]*100 + data[1]*10 + data[2];
	else if(RsAB == 1)
		databack = data[0]*100000 + data[1]*10000 + data[2]*1000 + data[3]*100 + data[4]*10 + data[5];	
	
	return databack;
}

int ascii2baud(uint8_t ch) {
	int baudback;
	switch(ch)
	{
		case '0':{ baudback = 4800; break; }
		case '1':{ baudback = 9600; break; }
		case '2':{ baudback = 14400; break; }
		case '3':{ baudback = 19200; break; }
		case '4':{ baudback = 28800; break; }
		case '5':{ baudback = 38400; break; }
		case '6':{ baudback = 56000; break; }
		case '7':{ baudback = 57600; break; }
		case '8':{ baudback = 115200; break; }
		case '9':{ baudback = 128000; break; }
		case 'A':{ baudback = 256000; break; } 
		default:{ baudback = 88; break; }
	}
	return baudback;
}

void push_data2web(USART_TypeDef *USARTx) {
	sendGSM("AT+HTTPINIT\n",50);
	sendGSM("AT+HTTPPARA=\"CID\",1\n",50);
	
	sendGSM("AT+HTTPPARA=\"URL\",\"http://phukhaco.teslateq.vn/sim.php?sr=1",0);	
	// Basic require
	sendGSM("&sig=",0);		Uart_putc(USARTx,staCSQ[0]);	Uart_putc(USARTx,staCSQ[1]);
	sendGSM("&cnn=",0);		Uart_putc(USARTx,web[0][0]);
	sendGSM("&b=",0);  		Uart_putc(USARTx,web[0][3]);	Uart_putc(USARTx,web[0][4]); 
	if(web[0][1] == '1' || loadBias <= numLoad) {
		//bias = AsciiToInt(web[0][3])*10 + AsciiToInt(web[0][4]);
		if(loadBias <= numLoad)
			loadBias++;
	}
	if(cntSum >= 92) {
		if(cntNEqual >=0 && cntNEqual < 100) {
			sendGSM("&cne=",0);				Uart_putc(USARTx,toAscii(cntNEqual/10));	Uart_putc(USARTx,toAscii(cntNEqual%10));
		}
	}
	if(simGPRS >= 1 && simGPRS <= 10) {
		sendGSM("&err=1",0);
		simGPRS++;
	}
	if(cntAlarm <= 10) {
		sendGSM("&dr=1",0);
		cntAlarm++;
	}
	sendGSM("&h=",0);  		GSM_hex(temp[0]);	GSM_hex(temp[1]);  GSM_hex(volt[0]);	GSM_hex(volt[1]); 
	
	// Service	
	// send 8in_2 and 4out_1
	sendGSM("&i2=",0);			GSM_hex(ADAM8024_in[0]);
	sendGSM("&o1=",0);			Uart_putc( USARTx, web[1][0] ); Uart_putc( USARTx, web[1][1] ); Uart_putc( USARTx, web[1][2] ); Uart_putc( USARTx, web[1][3] );
	
	tempVxl = adc[1]*51.0/4095.0;
	if(tempVxl < 6)
		sendGSM("&p=1",0);
	
	sendGSM("\"\n",50);
	sendGSM("AT+HTTPACTION=0\n",50);			
			
	// Read signal
	sendGSM("AT+CSQ\n",100);
	
	// Rst	GPRS
	sendGSM("AT+CGATT?\n",100);	
	if(staCgatt[0] != 49){
		stepConn = 0;
		simGPRS = 1;
		loadBias = 0;
	}
	
	Delay(50);
	
	
	process();
}

void process(void) {
	read_temp();
	read_volt();
	read_write_adam8024();
}
void read_temp(void) {
	temp[0] = adc[0]>>8;
	temp[1] = adc[0];
}
void read_volt(void) {
	volt[0] = adc[1]>>8;
	volt[1] = adc[1];
}
void read_write_adam8024(void) {

	if(web[1][0] == '0')			DO[0] = 0;
	else if(web[1][0] == '1')	DO[0] = 1;

	if(web[1][1] == '0')			DO[1] = 0;
	else if(web[1][1] == '1')	DO[1] = 1;
	
	if(web[1][2] == '0')			DO[2] = 0;
	else if(web[1][2] == '1')	DO[2] = 1;
	
	if(web[1][3] == '0')			DO[3] = 0;
	else if(web[1][3] == '1')	DO[3] = 1;
	
	uint8_t temp[1];		
	for(int i=0 ; i<4; i++)					//add data
		temp[0] = temp[0]<<1 | DO[i];
	ADAM8024_out[0] = temp[0]&0x0f ;
	Send_uart_data15(USART1,0x01,0x00,0x00,0x00,0x04, ADAM8024_out);
	
	Send_uart_data02(USART1,0x01,0x00,0x00,0x00,0x08, ADAM8024_in);			
}

void init_value(void) {
	// Add '0' to all
//	for(int i=0; i<rowW; i++)	{
//		for(int j=0; j<colW; j++)
//			web[i][j] = '0';
//	}
	// Enable rst GL
	for(int i=0; i<10; i++)
		web[0][i] = '0';
	for(int i=0; i<10; i++)
		web[1][i] = '0';
	web[0][2] = '1';
	
	// Bias
	bias = 10;
	
	loadBias = 0;
	
}
void daily_rst(void) {
	tellTime();
	tellDate();
	if(rst == 1 || (aShowTime[0] == 23 && aShowTime[1] == 59 && (aShowTime[2] >= 00 && aShowTime[2] <= 2))   )
	{
		Delay(1000);
		NVIC_SystemReset();
	}		
}
void cne_rst(void) {
	cntSum++;
	if(oldRead == web[0][0])
		cntEqual++;
	else
		cntNEqual++;
	oldRead = web[0][0];
	
	if(cntSum == 100) {					//init bias + read[4]
		if(cntNEqual <= bias) {
			//if(web[0][2] == '1')
				NVIC_SystemReset();
		}
		cntSum = 0;
		cntEqual = 0;
		cntNEqual = 0;
	}	
}

void del_all_mess(void) {
	GSMD("AT+CMGDA=\"DEL ALL\"\n",1000);
	for(int i=0; i<20; i++) {
		sms_phone[i] = 0;
		sms_mess[i] = 0;
		for(int j=0; j<20; j++)
			sms_mess_split[i][j] = 0;
		num_sdt = 0;
		len_sdt = 0;
	}
}
uint8_t check_phone(volatile char *data1, volatile char *str, uint16_t num) {
	for(uint16_t i; i<num; i++) {
		if(data1[i] != str[i])
			return 1;
	}
	return 0;
}
void sms_process(void) {
	if( strcmp((char*)sms_phone, "\"+84902555793\"") == 0 ) {						// if sdt: TRUE
			passCheck = 1;
			
		if(strcmp((char*)sms_mess, "*rst#") == 0) {															// rst
			passCheck = 2;
			
			enaSms[0] = 1;
			I2C_BufferWrite(I2C2, EE_Write, 0x01, enaSms, 1);
			
			// respone Ok
			GSM("AT+CMGS=\""); Uart_puts(USART6,sdt[0]); GSMD("\"\n",50);
			GSMD("rst: Ok",50);
			Uart_putc(USART6,(char)26); Delay(5000);
			IWDG_ReloadCounter();
			del_all_mess();
		}
		else if(strcmp((char*)sms_mess_split[1], "ds") == 0) {									// change phone
			passCheck = 3;
			num_sdt = as2i(sms_mess_split[2][0]);
			if(num_sdt != 0) {
				for(int i=0; i<len_sdt; i++) 
					sdt[num_sdt][i] = sms_mess_split[3][i];
				
				I2C_BufferWrite(I2C2, EE_Write, (num_sdt+2), (unsigned char*)sdt[num_sdt], len_sdt);
				
				GSM("AT+CMGS=\""); Uart_puts(USART6,sdt[0]); 	GSMD("\"\n",50);
				GSMB((unsigned char*)sms_mess_split[2], 1); 	GSM("-");					// num_sdt,
				GSMB((unsigned char*)sdt[num_sdt], len_sdt); 	GSMD(": Ok",70);	// sdt: Ok
				Uart_putc(USART6,(char)26); Delay(5000);
				IWDG_ReloadCounter();
			}
			del_all_mess();
		}
		else
			del_all_mess();
	}
	else if( check_phone(sms_phone,"\"+84",4) == 0 ) {									// if sdt: FALSE
		del_all_mess();
		passCheck1 = 1;
	}
}

void sms_warning(void) {
	for(int i=0; i<num_ena_sdt; i++) {
		GSM("AT+CMGS=\""); Uart_puts(USART6,sdt[i]); GSMD("\"\n",50);
		GSMD("Pin mode: On",100);
		Uart_putc(USART6,(char)26); Delay(5000);
		IWDG_ReloadCounter();
	}
}
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
