/*
 * user_main.c
 *
 *  Created on: Dec 29, 2019
 *      Author: George
 */

#include "stdio.h"
#include "string.h"
#include "main.h"
#include "console.h"
#include "datetime.h"
#include "beeper.h"
#include "LCD.h"
#include "test_temp.h"

#include "user_main.h"

static int but1_req=0,but2_req=0;


//-------------------------------------------------------------------------
static void LCD_update_time(COLOR col){
	char *date;
	date = get_time_string();
	LCD_DisplayString(5,100,date,&Font20,col,GREEN);
}

//-------------------------------------------------------------------------
static void LCD_main_screen(void){
	int ret;
	float temp;
	char temp_str[5];
	char *date_buf;
	//get temper
	test_onewire();
	ret=get_temperature(&temp);
	if(ret==_OK)
		sprintf(temp_str,"%04.1f",temp);
	else
		sprintf(temp_str,"--.-");
	//draw screen
	LCD_DisplayString(5,5,"Temperature",&Font12,LCD_BACKGROUND,YELLOW);
	LCD_DisplayString(5,20,temp_str,&Font24,LCD_BACKGROUND,RED);
	LCD_DisplayString(5,45,"Date",&Font12,LCD_BACKGROUND,YELLOW);
	date_buf = get_date_string();
	LCD_DisplayString(5,60,date_buf,&Font20,LCD_BACKGROUND,GREEN);
	LCD_DisplayString(5,80,"Time",&Font12,LCD_BACKGROUND,YELLOW);
	date_buf = get_time_string();
	LCD_DisplayString(5,100,date_buf,&Font20,BLUE,GREEN);
}

//-------------------------------------------------------------------------
void one_sec_time_event(void){
	static int sec_cnt=0;

	LCD_update_time(BLUE);

	if(sec_cnt==0){
		LCD_Clear(BLUE);
		LCD_main_screen();
	}

	if(sec_cnt==10){
		LCD_DisplayImage(0,0,&Image1);
		LCD_main_screen();
	}

	if(sec_cnt==20){
		LCD_DisplayImage(0,0,&Image2);
		LCD_main_screen();
	}

	if(sec_cnt==30){
		sec_cnt=0;
		return;
	}

	sec_cnt++;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void main_init(void){
	printf(HELLO_STR "\n");
	init_datetime();
	HAL_TIM_PWM_Start(&htim11,TIM_CHANNEL_1);
	LCD_Init(D2U_L2R);
	LCD_Demo();
	init_console();
}


void main_loop(void){
	static int cnt = 0;
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	HAL_Delay(100);

	if(command_req)
		parse_cmd();

	if(but1_req){
		beep(3000,100); //3KHz
		but1_req=0;
	}

	if(but2_req){
		beep(1000,100); //1KHz
		HAL_Delay(100);
		beep(1000,100); //1KHz
		but2_req=0;
	}

	if(!(cnt%10) && (cnt>0)){
		one_sec_time_event();
		cnt=0;
		return;
	}

	cnt++;
}


void button1_irq(void){
	but1_req=1;
}

void button2_irq(void){
	but2_req=1;
}

