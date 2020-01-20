/*
 * user_main.c
 *
 *  Created on: Dec 29, 2019
 *      Author: George
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "main.h"
#include "console.h"
#include "datetime.h"
#include "beeper.h"
#include "LCD.h"
#include "test_temp.h"
#include "menu.h"

#include "user_main.h"

static int but1_req=0,but2_req=0;

int menu_on=0;
int menu_id;

#define MENU_TIMEOUT 10
int mnu_tmout_cnt=0;

//-------------------------------------------------------------------------
// TEMP-DATE-TIME SCREEN
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
//MENUS
//-------------------------------------------------------------------------
#define MENU1			0

#define MNU1_TEST_BEEP	1
#define MNU1_EXIT		2

//-------------------------------------------------------------------------
static void create_menu1(){
	menu_id=MENU1;
	add_mnu_option(menu_id,"Item1");
	add_mnu_option(menu_id,"Beeper Test");
	add_mnu_option(menu_id,"Exit");
	set_mnu_item(menu_id,0);
	LCD_Clear(BLUE);
	show_menu(menu_id,BLUE);
}

//-------------------------------------------------------------------------
static void menu_exit(void){
	delete_all_menu();
	menu_on=0;
}

//-------------------------------------------------------------------------
static void menu_action(int mnu_id, int item_id){
	switch(mnu_id){
	case MENU1:
		switch(item_id){
		case MNU1_EXIT:
			menu_exit();
			break;
		case MNU1_TEST_BEEP:
			LCD_Clear(BLUE);
			show_info("Play Beeps!",MAGENTA);
			play_beeper_demo();
			menu_exit();
			break;
		}
		break; //MENU_1
	}
}

//-------------------------------------------------------------------------
static void menu_one_sec(void){
	mnu_tmout_cnt++;
	if(mnu_tmout_cnt>MENU_TIMEOUT){
		delete_all_menu();
		menu_on=0;
	}
}


//-------------------------------------------------------------------------
//BUTTONS
//-------------------------------------------------------------------------
static void button_menu(void){
	mnu_tmout_cnt=0;
	if(!menu_on){
		create_menu1();
		menu_on=1;
	}
	else{
		set_mnu_item_next(menu_id);
		show_menu(menu_id,BLUE);
	}
}



//-------------------------------------------------------------------------
static void button_select(void){
	if(!menu_on)
		return;
	menu_action(menu_id,get_mnu_item(menu_id));
}


//-------------------------------------------------------------------------
//ONE_SEC
//-------------------------------------------------------------------------
static void one_sec_time_event(void){
	static int sec_cnt=0;

	if(menu_on){
		menu_one_sec();
		sec_cnt=0;
		return;
	}

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
//MAIN
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
		button_select();
		but1_req=0;
	}

	if(but2_req){
		beep(1000,100); //1KHz
		button_menu();
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

