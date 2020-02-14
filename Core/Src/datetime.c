/*
 * datetime.c
 *
 *  Created on: 17 ��� 2017
 *      Author: gkyr
 */

#include "stm32f4xx_hal.h"
#include "datetime.h"
#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "string.h"
#include "main.h"

#define BACKUP_REGS 20
#define BACKUP_SIZE (BACKUP_REGS*4) //80 bytes

typedef struct back_data{
	OPTIONS options; //up to 20 bytes
	U8 reserv[BACKUP_SIZE-USERSTR_SIZE-sizeof(OPTIONS)];
	char user_str[USERSTR_SIZE];
}PACKED BACK_DATA;  //size of backup data = BACKUP_REGS*4 = 80bytes

typedef union{
	U32  regs[BACKUP_REGS];
	BACK_DATA data;
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
int enable_alarm(int alrm_id, int enable){
	RTC_AlarmTypeDef sAlarm;
	uint32_t alarm;

	if(alrm_id==1)
		alarm=RTC_ALARM_A;
	else
		alarm=RTC_ALARM_B;

	if(enable){
		if(HAL_RTC_GetAlarm(&hrtc,&sAlarm,alarm,RTC_FORMAT_BIN)==HAL_OK)
			HAL_RTC_SetAlarm_IT(&hrtc,&sAlarm,RTC_FORMAT_BIN);
	}
	else{
		if(HAL_RTC_DeactivateAlarm(&hrtc,alarm)!=HAL_OK)
			return _ERR;
	}
	return _OK;
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

	sprintf(datetime_buff,"-/-/--");

	if(!date_init){
		return datetime_buff;
	}

	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		 printf("fail read date!");
		 return datetime_buff;
	 }


	 sprintf(datetime_buff,"%d/%d/20%d",sDate.Date,sDate.Month,sDate.Year);
	 return datetime_buff;
}

//-----------------------------------------------------------
char* get_day_string(void){
	RTC_DateTypeDef sDate;

	sprintf(datetime_buff,"-----");

	if(!date_init){
		return datetime_buff;
	}

	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		 printf("fail read date!");
		 return datetime_buff;
	 }

	 switch(sDate.WeekDay){
	 case RTC_WEEKDAY_MONDAY:   return "Monday";
	 case RTC_WEEKDAY_TUESDAY:  return "Tuesday";
	 case RTC_WEEKDAY_WEDNESDAY:return "Wednesday";
	 case RTC_WEEKDAY_THURSDAY: return "Thursday";
	 case RTC_WEEKDAY_FRIDAY:   return "Friday";
	 case RTC_WEEKDAY_SATURDAY: return "Saturday";
	 case RTC_WEEKDAY_SUNDAY:   return "Sunday";
	 }

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

	 sprintf(datetime_buff,"%02d:%02d:%02d",sTime.Hours,sTime.Minutes,sTime.Seconds);

	 if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK)
		 printf("Error getDate\n");

	 return datetime_buff;
}


//-----------------------------------------------------------
int set_time_args(char *hour, char *min, char *sec){
	RTC_TimeTypeDef sTime;
	int value;

	memset(&sTime,0,sizeof(RTC_TimeTypeDef));
	value=atoi(hour);
	if((value<0) || (value>23))
		return _ERR;
	sTime.Hours=value;

	value=atoi(min);
	if((value<0) || (value>59))
		return _ERR;
	sTime.Minutes=value;

	value=atoi(sec);
	if((value<0) || (value>59))
		return _ERR;
	sTime.Seconds=value;

	if (HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN) != HAL_OK){
		printf("fail set time!\n");
		return _ERR;
 	}

	return _OK;
}


//-----------------------------------------------------------
int set_date_args(char *day, char *date, char *month, char *year){
	RTC_DateTypeDef sDate;
	int value;

	memset(&sDate,0,sizeof(RTC_DateTypeDef));
	value=atoi(day);
	if((value<1) || (value>7))
		return _ERR;
	sDate.WeekDay=value;

	value=atoi(date);
	if((value<1) || (value>31))
		return _ERR;
	sDate.Date=value;

	value=atoi(month);
	if((value<1) || (value>12))
		return _ERR;
	sDate.Month=value;

	value=atoi(year);
	if((value<0) || (value>99))
		return _ERR;
	sDate.Year=value;

	if (HAL_RTC_SetDate(&hrtc,&sDate,RTC_FORMAT_BIN) != HAL_OK){
		printf("fail set date!");
		return _ERR;
	}

	return _OK;
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
int set_alarm_args(int alrm, char *date, char *hour, char *min, char *sec){
	RTC_AlarmTypeDef sAlarm = {0};

	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;

	if(alrm==1)
		sAlarm.Alarm=RTC_ALARM_A;
	else if(alrm==2)
		sAlarm.Alarm=RTC_ALARM_B;
	else
		return _ERR;

	if(!strcmp("-",date))
		sAlarm.AlarmMask |= RTC_ALARMMASK_DATEWEEKDAY;
	else
		sAlarm.AlarmDateWeekDay = atoi(date);

	if(!strcmp("-",hour))
		sAlarm.AlarmMask |= RTC_ALARMMASK_HOURS;
	else
		sAlarm.AlarmTime.Hours= atoi(hour);

	if(!strcmp("-",min))
		sAlarm.AlarmMask |= RTC_ALARMMASK_MINUTES;
	else
		sAlarm.AlarmTime.Minutes= atoi(min);

	if(!strcmp("-",sec))
		sAlarm.AlarmMask |= RTC_ALARMMASK_SECONDS;
	else
		sAlarm.AlarmTime.Seconds=atoi(sec);

	if(HAL_RTC_SetAlarm_IT(&hrtc,&sAlarm,RTC_FORMAT_BIN)!=HAL_OK)
		return _ERR;

	return _OK;
}

//-----------------------------------------------------------
static void parse_alarm(RTC_AlarmTypeDef *sAlarm, char *alrm_buff){
	char enbl[4], date[3],hour[3],min[3],sec[3];
	uint32_t src_alrm;

	if(sAlarm->Alarm==RTC_ALARM_A)
		src_alrm=RTC_IT_ALRA;
	else
		src_alrm=RTC_IT_ALRB;
	if(__HAL_RTC_ALARM_GET_IT_SOURCE(&hrtc,src_alrm))
		sprintf(enbl,"on");
	else
		sprintf(enbl,"off");

	if(sAlarm->AlarmMask & RTC_ALARMMASK_DATEWEEKDAY)
		sprintf(date,"--");
	else
		sprintf(date,"%02d",sAlarm->AlarmDateWeekDay);
	if(sAlarm->AlarmMask & RTC_ALARMMASK_HOURS)
		sprintf(hour,"--");
	else
		sprintf(hour,"%02d",sAlarm->AlarmTime.Hours);
	if(sAlarm->AlarmMask & RTC_ALARMMASK_MINUTES)
		sprintf(min,"--");
	else
		sprintf(min,"%02d",sAlarm->AlarmTime.Minutes);
	if(sAlarm->AlarmMask & RTC_ALARMMASK_SECONDS)
		sprintf(sec,"--");
	else
		sprintf(sec,"%02d",sAlarm->AlarmTime.Seconds);

	sprintf(alrm_buff,"%3s. [%s] %s:%s:%s",enbl, date,hour,min,sec);
}

//-----------------------------------------------------------
int get_alarms(char *alrm1, char *alrm2){
	RTC_AlarmTypeDef sAlarm1,sAlarm2;

	if(HAL_RTC_GetAlarm(&hrtc,&sAlarm1,RTC_ALARM_A,RTC_FORMAT_BIN)!=HAL_OK){
		printf("fail get alarm1!");
		return _ERR;
	}
	if(HAL_RTC_GetAlarm(&hrtc,&sAlarm2,RTC_ALARM_B,RTC_FORMAT_BIN)!=HAL_OK){
		printf("fail get alarm2!");
		return _ERR;
	}

	parse_alarm(&sAlarm1,alrm1);
	parse_alarm(&sAlarm2,alrm2);
	return _OK;
}

//-----------------------------------------------------------
void write_backup_str(char *data){
	strncpy(backup.data.user_str,data,sizeof(backup.data.user_str));
	for(int ii=0; ii<BACKUP_REGS; ii++)
		HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0+ii,backup.regs[ii]);
}

//-----------------------------------------------------------
char *read_backup_str(void){
	for(int ii=0; ii<BACKUP_REGS; ii++)
		backup.regs[ii] = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0+ii);
	backup.data.user_str[sizeof(backup.data.user_str)-1]=0;
	return backup.data.user_str;
}

//-----------------------------------------------------------
void write_options(OPTIONS *options){
	if(!memcmp(options,&backup.data.options,sizeof(OPTIONS)))
		return;
	memcpy(&backup.data.options,options,sizeof(OPTIONS));
	for(int ii=0; ii<BACKUP_REGS; ii++)
		HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0+ii,backup.regs[ii]);
}

//-----------------------------------------------------------
void read_options(OPTIONS *options){
	for(int ii=0; ii<BACKUP_REGS; ii++)
		backup.regs[ii] = HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0+ii);
	memcpy(options,&backup.data.options,sizeof(OPTIONS));
}

