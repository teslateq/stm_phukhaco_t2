#ifndef _Sim800A_H_
#define _Sim800A_H_
#include "stm32f4xx.h"
#include "basicFunc.h"

// err ~ simEna
// Sim_err ~ simGPRS
// stepConn - statusM95 - staM95

extern uint8_t stepConn;
extern uint8_t statusM95;
extern uint8_t simGPRS;
extern uint8_t simEna;
extern volatile uint8_t staM95;
extern volatile int16_t pos;
extern volatile char buffer[300];

// Take from server
#define rowW 30
#define colW 10
extern volatile char web[rowW][colW];


extern volatile uint8_t staCreg[5];
extern volatile uint8_t staCgatt[5];

extern volatile uint8_t staCSQ[5];
extern volatile uint8_t Signal;
extern volatile uint8_t cntLostSignal;
extern volatile uint8_t cntLS;
extern volatile uint8_t deSignal;


extern volatile char sms_phone[50];
extern volatile char sms_mess[200];
extern volatile char sms_mess_split[50][50];
extern volatile uint8_t pos_phone, pos_mess;
extern volatile uint16_t sms_row, sms_col, num_sdt, len_sdt;


//Modbus OTA receive
extern volatile char Modbus_master_slave;
extern volatile char Modbus_master_add[3];
extern volatile char Modbus_slave_add[3];
extern volatile char Modbus_master_baud;
extern volatile char Modbus_slave_baud;
extern volatile char Modbus_master_addmi[4];
extern volatile char Modbus_master_len[4];
extern volatile uint8_t Modbus_master_wr_add[3];
extern volatile uint8_t Modbus_master_wr_addmi[4];
extern volatile uint8_t Modbus_master_wr_len[4];
extern volatile uint8_t Modbus_master_wr_data[50]; //len max la 25


extern void Delay(__IO uint32_t nCount);
extern void Uart_puts(USART_TypeDef *USARTx,volatile char *str);
extern void Uart_putc(USART_TypeDef *USARTx,uint8_t ch);
extern uint8_t AsciiToInt(uint8_t ch);
extern uint8_t hex2asc(uint8_t ch);
void USART6_Configuration(unsigned int BaudRate);
void sendGSM(volatile char* msg, int waitMs);
void GSM(volatile char* msg);
void GSMD(volatile char* msg, int waitMs);
void GSMB(uint8_t* msg, uint8_t len);
void GSM_hex(uint8_t data);
void GSMH(uint8_t* data, uint32_t len);
void resetBuffer(void);
void USART6_IRQHandler(void);
int ascii2baud(uint8_t ch);
void PWK_On(void);
void PWK_Off(void);
void check_statusM95(void);
void check_statusM95_main(void);
void check_signal(void);
void check_gprs(void);


#endif   
