/*
 * datetime.h
 *
 *  Created on: 17 ��� 2017
 *      Author: gkyr
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include "types.h"
#include "user_main.h" //OPTIONS

typedef struct
{
    U8 second; // 0-59
    U8 minute; // 0-59
    U8 hour;   // 0-23
    U8 day;    // 1-31
    U8 month;  // 1-12
    U8 year;   // 0-99 (representing 2000-2099)
}
date_time_t;

void init_datetime(void);
void set_datetime(U32 epoch);

int get_datetime_struct(date_time_t* date_time);
char* get_datetime_string(void);
char* get_day_string(void);
char* get_date_string(void);
char* get_time_string(void);

int set_time_args(char *hour, char *min, char *sec);
int set_date_args(char *day, char *date, char *month, char *year);

int enable_alarm(int alrm_id, int enable);
int set_alarm_args(int alrm, char *date, char *hour, char *min, char *sec);
int get_alarms(char *alrm1, char *alrm2);

U32 get_datetime_epoch(void);
char* get_epoch_string(date_time_t* date_time);

extern U32 date_time_to_epoch(date_time_t* date_time);
extern void epoch_to_date_time(date_time_t* date_time,U32 epoch);

extern void write_backup_str(char *data);
extern char *read_backup_str(void);

void read_options(OPTIONS *options);
void write_options(OPTIONS *options);


#endif /* DATETIME_H_ */
