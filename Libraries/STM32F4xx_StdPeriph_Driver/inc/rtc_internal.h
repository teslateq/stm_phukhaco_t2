#ifndef __rtc_internal_H
#define __rtc_internal_H
#include "stm32f4xx.h"
/* 
	- misc.c
	- stm32f4xx_rcc.c
	- stm32f4xx_pwr.c
	- stm32f4_rtc.c
	
	RTC_AlarmTypeDef 	RTC_AlarmStructure;		//not use yet
*/

#define RTC_CLOCK_SOURCE_LSE

extern uint8_t aShowTime[10];


void setTime(uint8_t hour, uint8_t min, uint8_t secs, uint8_t date, uint8_t month, uint16_t year, uint8_t week);
void init_RTC(uint8_t hour, uint8_t min, uint8_t secs, uint8_t date, uint8_t month, uint16_t year, uint8_t week);
void tellTime(void);
void tellDate(void);








#endif




