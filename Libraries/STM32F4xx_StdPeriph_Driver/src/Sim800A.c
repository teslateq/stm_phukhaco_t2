#include <stm32f4xx.h>
#include "Sim800A.h"

uint8_t stepConn = 0;
uint8_t statusM95;
uint8_t simGPRS;
int simDr = 1;
uint8_t simEna;
volatile uint8_t staM95;
volatile int16_t pos = 0;
volatile char buffer[300];

// Take from server
volatile char web[rowW][colW];
volatile int row,col;

enum _parseStateSim {
  PS_DETECT_MSG_TYPE,
	PS_CHECK_STA,
	PS_CHECK_OK,
  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT,
	
	PS_CREG_N,
	PS_CREG_STAT,
	
	PS_CGATT_STAT,
	
	PS_CSQSpace,
	PS_CSQ,
	
	PS_CMGR_READ,
	PS_CMGR_PHONE,
	PS_CMGR_NULL_DATETIME,
	PS_CMGR_MESSAGE
};
volatile uint8_t parseStateSim = PS_DETECT_MSG_TYPE;


//Modbus OTA receive
volatile uint8_t staCreg[5];
volatile uint8_t staCgatt[5];
volatile uint8_t staCSQ[5];
volatile uint8_t Signal;
volatile uint8_t cntLostSignal;
volatile uint8_t cntLS;
volatile uint8_t deSignal;


volatile char sms_phone[50];
volatile char sms_mess[200];
volatile char sms_mess_split[50][50];
volatile uint8_t pos_phone, pos_mess;
volatile uint16_t sms_row, sms_col, num_sdt, len_sdt;


volatile char Modbus_master_slave;
volatile char Modbus_master_add[3];
volatile char Modbus_slave_add[3];
volatile char Modbus_master_baud;
volatile char Modbus_slave_baud;
volatile char Modbus_master_addmi[4];
volatile char Modbus_master_len[4];
volatile uint8_t Modbus_master_wr_add[3];
volatile uint8_t Modbus_master_wr_addmi[4];
volatile uint8_t Modbus_master_wr_len[4];
volatile uint8_t Modbus_master_wr_data[50]; //len max la 25



void USART6_Configuration(unsigned int BaudRate)
{	//for sim
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	//E2 for PWK
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
  /* Configure USART Tx as alternate function  */ //B6:Tx  B7:Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6); 
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); 
	
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(USART6, &USART_InitStructure);
  
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
  USART_Cmd(USART6, ENABLE);
}

void sendGSM(volatile char* msg, int waitMs) {
	Uart_puts(USART6,msg);
	Delay(waitMs);
}
void GSM(volatile char* msg) {
	Uart_puts(USART6, msg);
}
void GSMD(volatile char* msg, int waitMs) {
	Uart_puts(USART6, msg);
	Delay(waitMs);
}

void GSMB(uint8_t* msg, uint8_t len) {
	for(int i=0; i<len; i++)
		Uart_putc(USART6, msg[i]);
}
void GSM_hex(uint8_t data) {
	uint8_t d1, d2;
	d1 = hex2asc( data >> 4 );
	d2 = hex2asc( data & 0x0f );
	Uart_putc(USART6, d1);
	Uart_putc(USART6, d2);
}
void GSMH(uint8_t* data, uint32_t len) {
	for(int i=0; i<len; i++)
		GSM_hex(data[i]);
}


void resetBuffer(void) {
  memset((char*)buffer, 0, sizeof(buffer));
  pos = 0;
}

void PWK_On(void){
	GPIO_SetBits(GPIOD,GPIO_Pin_12);
}
void PWK_Off(void){
	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
}


void check_statusM95(void) {
		staM95 = 0;
		sendGSM("AT\n",100);
		statusM95 = staM95;
		if(statusM95 == 0) {
			simEna++;
			PWK_On();
			Delay(1600);
			PWK_Off();
			Delay(1000);
		}
		else if(statusM95 == 1) {
			PWK_On();
			Delay(1500);
			PWK_Off();
			Delay(3000);
			PWK_On();
			Delay(1500);
			PWK_Off();
			Delay(100);
		}
}

void check_statusM95_main(void) {
		staM95 = 0;
		sendGSM("AT\n",100);
		statusM95 = staM95;
		if(statusM95 == 0) {
			simEna++;
			PWK_On();
			Delay(1500);
			PWK_Off();
			Delay(100);
		}
		staM95 = 0;
		sendGSM("AT\n",100);
		statusM95 = staM95;
		if(statusM95 == 1)
			stepConn = 1;
}
void check_signal(void) {
	sendGSM("AT+CREG?\n",3000);
	if(staCreg[1] == 49)
		stepConn = 2;
}
void check_gprs(void) {
	sendGSM("AT+SAPBR=3,1,\"APN\",\"m-wap\"\n",500);
	sendGSM("AT+SAPBR=1,1\n",1000);
	sendGSM("AT+CGATT?\n",500);	
	if(staCgatt[0] == 49) {
		Delay(5000);
		stepConn = 3;
	}
}

void USART6_IRQHandler(void) //for NVIC_RXNE
{
	//-------------------------------------------------Start
	if( USART_GetITStatus(USART6, USART_IT_RXNE))
	{
		char b = USART6->DR;
		buffer[pos++] = b;
		
		if ( pos >= sizeof(buffer) )
			resetBuffer(); // just to be safe
		
		switch(parseStateSim){
			case PS_DETECT_MSG_TYPE: 
				{
					if ( b == '\n' )	//01 03 00 00 00 02 c4 0b
						resetBuffer();
					else {
						if ( pos == 3 && strcmp((char*)buffer, "AT+") == 0 ) {
							parseStateSim = PS_IGNORING_COMMAND_ECHO;
						}
						else if ( b == ':' ) {
							if ( strcmp((char*)buffer, "+HTTPACTION:") == 0 ) {
								parseStateSim = PS_HTTPACTION_TYPE;
							}
							else if ( strcmp((char*)buffer, "+HTTPREAD:") == 0 ) {           
								parseStateSim = PS_HTTPREAD_LENGTH;
							}
							else if ( strcmp((char*)buffer, "+CREG:") == 0) {
								parseStateSim = PS_CREG_N;
							}
							else if ( strcmp((char*)buffer, "+CGATT:") == 0) {
								parseStateSim = PS_CGATT_STAT;
							}
							else if ( strcmp((char*)buffer, "+CSQ:") == 0) {
								parseStateSim = PS_CSQSpace;
							}
							else if ( strcmp((char*)buffer, "+CMGR:") == 0) {
								parseStateSim = PS_CMGR_READ;
							}
							resetBuffer();
						}
						
						if ( pos == 3 && strcmp((char*)buffer, "AT\r") == 0 ) {
							parseStateSim = PS_CHECK_STA;
						}
					}
				}
				break;
		
			case PS_CHECK_STA:
				{
					if ( b == '\n' ){
						staM95 = 1;
						parseStateSim = PS_CHECK_OK;
						resetBuffer();
					}
				}
				break;	
				
			case PS_CHECK_OK:
				{
					if(b == '\n'){
						if(strcmp((char*)buffer, "OK") == 0){
							staM95 = 1;
						}
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
				
			case PS_IGNORING_COMMAND_ECHO:
				{
					if ( b == '\n' ){
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
		
			case PS_HTTPACTION_TYPE:
				{
					if ( b == ',' ){
						parseStateSim = PS_HTTPACTION_RESULT;
						resetBuffer();
					}
				}
				break;
		
			case PS_HTTPACTION_RESULT:
				{
					if ( b == ',' ){
						parseStateSim = PS_HTTPACTION_LENGTH;
						resetBuffer();
					}
				}
				break;
		
			case PS_HTTPACTION_LENGTH:
				{
					if ( b == '\n' ) {
						// Serial.print("HTTPACTION length is ");
						// Serial.println(buffer);
						// if(buffer!='5')
						//   loi=1;
					 // now request content
						Uart_puts(USART6,"AT+HTTPREAD=0,");
						Uart_puts(USART6,buffer);
						Uart_puts(USART6,"\n");
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
		
			case PS_HTTPREAD_LENGTH:
				{
					if ( b == '\n' ){  
						parseStateSim = PS_HTTPREAD_CONTENT;
						resetBuffer();
					}
				}
				break;
																									// Di | Active | Madd | Mbaud | Maddmi | Mlen | Sadd | Sbaud 
			case PS_HTTPREAD_CONTENT:										// 000 1 0000 00 0  0  0	0	 0	0  0  0  0  0
				{																	// 012 3 4567 89.10.11 12.13.14.15 16.17.18.19
					if( b == '\n') {
						row = 0;
						col = 0;
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
					else if( b == ',' ) {
						row++;
						col = 0;
					}
					else if( b == 10 ) {}			//do nothing, to clear 10 previous
					else {
						if( b == 13 )	{}				//do nothing to clear 13
						else {
							web[row][col] = b;
							col++;
						}
					}
				}
				break;
				
			case PS_CREG_N:
				{
					if ( b == ',' ){
						staCreg[0] = (uint8_t)buffer[1];
						parseStateSim = PS_CREG_STAT;
						resetBuffer();
					}
				}
				break;
				
			case PS_CREG_STAT:
				{
					if ( b == '\n' ){
						staCreg[1] = (uint8_t)buffer[0];
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
				
			case PS_CGATT_STAT:
				{
					if ( b == '\n' ){
						staCgatt[0] = (uint8_t)buffer[1];
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
				
			case PS_CSQSpace:
				{
					if ( b == 32 ){
						parseStateSim = PS_CSQ;
						resetBuffer();
					}
				}
				break;
				
			case PS_CSQ:
				{
					if ( b == ',' ){
						for(int i=0; i<pos-1; i++)
							staCSQ[i] = buffer[i];
						Signal = atoi((char*)staCSQ);
						parseStateSim = PS_IGNORING_COMMAND_ECHO;
						resetBuffer();
					}
				}
				break;
				
			case PS_CMGR_READ:
			{
				if(b == ',') {
					parseStateSim = PS_CMGR_PHONE;
					resetBuffer();
				}
			}
			break;
			
			case PS_CMGR_PHONE:
			{
				if(b == ',') {
//					for(int i=0; i<20; i++)
//						sms_phone[i] = buffer[i];
					parseStateSim = PS_CMGR_NULL_DATETIME;
					resetBuffer();
					pos_phone = 0;
				}
				else {
					sms_phone[pos_phone] = buffer[pos_phone];
					pos_phone++;
				}
				
			}
			break;
			
			case PS_CMGR_NULL_DATETIME:
			{
				if(b == '\n') {
					parseStateSim = PS_CMGR_MESSAGE;
					resetBuffer();
				}
			}
			break;
			
			case PS_CMGR_MESSAGE:
			{
				if(b == '\n') {
					len_sdt = sms_col;
					
					parseStateSim = PS_DETECT_MSG_TYPE;
					resetBuffer();
					pos_mess = 0;
					sms_row = 0;
					sms_col = 0;
				}
				else if(b == 13) {}
				else {
					sms_mess[pos_mess] = buffer[pos_mess];
					pos_mess++;
					
					if(b == '*') {
						sms_row++;
						sms_col = 0;
					}
					else {
						sms_mess_split[sms_row][sms_col] = b;
						sms_col++;
					}
				}
			}
			break;
				
				
			default:
				break;
		}
	}
}
