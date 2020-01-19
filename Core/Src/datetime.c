/*
 * datetime.c
 *
 *  Created on: 17 ��� 2017
 *      Author: gkyr
 */

#include "stm32f4xx_hal.h"
#include "datetime.h"
#include "stdio.h"
#include "string.h"
#include "main.h"

#define BACKUP_REGS 20
typedef union{
	char buf8[BACKUP_REGS*4];
	U32  buf32[BACKUP_REGS];
}BACK_UP;


static BACK_UP backup;

static char datetime_buff[30]="-";
static BOOL date_init=FALSE;

#define EPOCH_2000	946684800

static U16 days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};

//-----------------------------------------------------------
U32 date_time_to_epoch(date_time_t* date_time)
{	U32 epoch_time;
    U32 second = date_time->second;  // 0-59
    U32 minute = date_time->minute;  // 0-59
    U32 hour   = date_time->hour;    // 0-23
    U32 day    = date_time->day-1;   // 0-30
    U32 month  = date_time->month-1; // 0-11
    U32 year   = date_time->year;    // 0-99
    epoch_time = (((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second;
    return epoch_time + EPOCH_2000;
}

//-----------------------------------------------------------
void epoch_to_date_time(date_time_t* date_time,U32 epoch)
{

	epoch = epoch - EPOCH_2000;
	date_time->second = epoch%60; epoch /= 60;
    date_time->minute = epoch%60; epoch /= 60;
    date_time->hour   = epoch%24; epoch /= 24;

    U32 years = epoch/(365*4+1)*4; epoch %= 365*4+1;

    U32 year;
    for (year=3; year>0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    U32 month;
    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = years+year;
    date_time->month = month+1;
    date_time->day   = epoch-days[year][month]+1;
}

//-----------------------------------------------------------
void init_datetime(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) == HAL_OK){
		if(sDate.Year!=0){
			date_init=TRUE;
			return;
		}
	 }

	memset(&sDate,0,sizeof(RTC_DateTypeDef));
	sDate.Year=10;
	sDate.Month=1;
	sDate.Date=1;
	if (HAL_RTC_SetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		printf("fail set date!");
		return;
	}

	memset(&sTime,0,sizeof(RTC_TimeTypeDef));
	if (HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN) != HAL_OK){
		printf("fail set time!");
		return;
 	}

	date_init=TRUE;
}

//-----------------------------------------------------------
char* get_datetime_string(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	U32 subsec;
	float subtime;

	if(!date_init){
  	    sprintf(datetime_buff,"%ld",HAL_GetTick());
		return datetime_buff;
	}

	 if (HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN) != HAL_OK){
		 printf("fail read time!");
		 return "?";
	 }
	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		 printf("fail read date!");
		 return "?";
	 }

	 subsec = sTime.SecondFraction - sTime.SubSeconds;
	 subtime = (float)subsec * ((float)1000/(float)sTime.SecondFraction);
	 subsec = (U16)subtime;
	 if(subsec>1) subsec--;

	 sprintf(datetime_buff,"20%d/%d/%d %d:%d:%d.%ld",sDate.Year,sDate.Month,sDate.Date,sTime.Hours,sTime.Minutes,sTime.Seconds,subsec);

	 return datetime_buff;
}

//-----------------------------------------------------------
char* get_date_string(void){
	RTC_DateTypeDef sDate;

	sprintf(datetime_buff,"--/-/-");

	if(!date_init){
		return datetime_buff;
	}

	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		 printf("fail read date!");
		 return datetime_buff;
	 }

	 sprintf(datetime_buff,"20%d/%d/%d",sDate.Year,sDate.Month,sDate.Date);
	 return datetime_buff;
}

//-----------------------------------------------------------
char* get_time_string(void){
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;

	sprintf(datetime_buff,"-.-.-");

	if(!date_init){
		return datetime_buff;
	}

	if (HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN) != HAL_OK){
		 return datetime_buff;
	 }

	 sprintf(datetime_buff,"%2d:%2d:%2d",sTime.Hours,sTime.Minutes,sTime.Seconds);

	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK)
		 printf("Error getDate\n");

	 return datetime_buff;
}

//-----------------------------------------------------------
U32 get_datetime_epoch(void){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	date_time_t time;

	 if (HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN) != HAL_OK){
		 printf("fail read time!");
		 return 0;
	 }
	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		 printf("fail read date!");
		 return 0;
	 }

	 time.year=sDate.Year;
	 time.month=sDate.Month;
	 time.day=sDate.Date;
	 time.hour=sTime.Hours;
	 time.minute=sTime.Minutes;
	 time.second=sTime.Seconds;

	 return date_time_to_epoch(&time);
}

//-----------------------------------------------------------
void set_datetime(U32 epoch){
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	date_time_t time;

	//add GMT +02:00
	epoch += 7200;

	printf("epoch=%p",(void*)epoch);
	epoch_to_date_time(&time,epoch);

	memset(&sDate,0,sizeof(RTC_DateTypeDef));
	sDate.Year=time.year;
	sDate.Month=time.month;
	sDate.Date=time.day;
	if (HAL_RTC_SetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		printf("fail set date!");
		return;
	}

	memset(&sTime,0,sizeof(RTC_TimeTypeDef));
	sTime.Hours=time.hour;
	sTime.Minutes=time.minute;
	sTime.Seconds=time.second;
	if (HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN) != HAL_OK){
		printf("fail set time!");
		return;
 	}

}

//-----------------------------------------------------------
void write_backup_str(char *data){
	strncpy(backup.buf8,data,sizeof(BACK_UP));
	for(int ii=0; ii<BACKUP_REGS; ii++)
		HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0+ii,backup.buf32[ii]);
}

//-----------------------------------------------------------
char *read_backup_str(void){
	for(int ii=0; ii<BACKUP_REGS; ii++)
		backup.buf32[ii] = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0+ii);
	backup.buf8[sizeof(BACK_UP)-1]=0;
	return backup.buf8;
}

