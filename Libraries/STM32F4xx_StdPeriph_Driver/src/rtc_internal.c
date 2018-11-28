#include "stm32f4xx.h"
#include "rtc_internal.h"


uint8_t aShowTime[10] = {0};

void setTime(uint8_t hour, uint8_t min, uint8_t secs, uint8_t date, uint8_t month, uint16_t year, uint8_t week){
	RTC_TimeTypeDef  	RTC_TimeStructure;
	RTC_DateTypeDef		RTC_DateStructure;
	
	RTC_TimeStructure.RTC_Hours   = hour;
	RTC_TimeStructure.RTC_Minutes = min;
	RTC_TimeStructure.RTC_Seconds = secs;
	RTC_DateStructure.RTC_Month = month;
	RTC_DateStructure.RTC_Date = date;
	RTC_DateStructure.RTC_Year = year;
	RTC_DateStructure.RTC_WeekDay = week;
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
}

void init_RTC(uint8_t hour, uint8_t min, uint8_t secs, uint8_t date, uint8_t month, uint16_t year, uint8_t week) {
	RTC_InitTypeDef   RTC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	RCC_LSEConfig(RCC_LSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET){}
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);

	RTC_WaitForSynchro();
	RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv =  0xFF;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);
		
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
	RTC_SetWakeUpCounter(0x7FF);
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	RTC_WakeUpCmd(ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
	
	setTime(hour, min, secs, date, month, year, week);
}



void tellTime(void){
	RTC_TimeTypeDef  	RTC_TimeStructure;
	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	aShowTime[0] = RTC_TimeStructure.RTC_Hours;
	aShowTime[1] = RTC_TimeStructure.RTC_Minutes;
	aShowTime[2] = RTC_TimeStructure.RTC_Seconds;
}

void tellDate(void){
	RTC_DateTypeDef		RTC_DateStructure;
	
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	aShowTime[3] = RTC_DateStructure.RTC_Date;
	aShowTime[4] = RTC_DateStructure.RTC_Month;
	aShowTime[5] = RTC_DateStructure.RTC_Year;
}


