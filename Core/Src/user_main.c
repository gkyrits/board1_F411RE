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
#include "fatfs_utils.h"

#include "user_main.h"

static int but1_req=0,but2_req=0;

static int menu_on=0;
static int menu_id;

#define IMG_TMOUT	20
int play_mode=PLYMOD_INF;
int internal_img=0;
static int play_mode_old=-1;

#define MENU_TIMEOUT 10
static int mnu_tmout_cnt=0;

//-------------------------------------------------------------------------
// TEMP-DATE-TIME SCREEN
//-------------------------------------------------------------------------
static void LCD_update_time(COLOR col){
	char *date;
	date = get_time_string();
	LCD_DisplayString(5,100,date,&Font20,col,GREEN);
}

//-------------------------------------------------------------------------
static void LCD_info_screen(void){
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
// IMAGE SCREEN
//-------------------------------------------------------------------------
static void LCD_image_screen(void){
	static int idx=0;
	int err;
	if(internal_img==0){
		err=LCD_load_next_image();
		if(err!=NO_ERR){
			internal_img=1;
		}
	}
	if(internal_img==1){
		if(idx==0)
			LCD_DisplayImage(0,0,&Image1);
		else if(idx==1)
			LCD_DisplayImage(0,0,&Image2);
		idx++;
		if(idx>1)
			idx=0;
	}
}

//-------------------------------------------------------------------------
//MENUS
//-------------------------------------------------------------------------
#define MENU1			0

#define MNU1_SLIDE_SD		0
#define MNU1_SLIDE_SD_INF	1
#define MNU1_SLIDE_INT_INF	2
#define MNU1_SLIDE_OFF		3
#define MNU1_TEST_BEEP		4
#define MNU1_EXIT			5

//-------------------------------------------------------------------------
static void create_menu1(){
	menu_id=MENU1;
	add_mnu_option(menu_id,"Slide sd");
	add_mnu_option(menu_id,"Slide sd/info");
	add_mnu_option(menu_id,"Slide int/info");
	add_mnu_option(menu_id,"Slide off");
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
	int err;
	switch(mnu_id){
	case MENU1:
		switch(item_id){
		case MNU1_SLIDE_SD:
			play_mode = PLYMOD_IMG;
			internal_img = 0;
			read_image_list(&err);
			menu_exit();
			break;
		case MNU1_SLIDE_SD_INF:
			play_mode = PLYMOD_IMG_INF;
			internal_img = 0;
			read_image_list(&err);
			menu_exit();
			break;
		case MNU1_SLIDE_INT_INF:
			play_mode = PLYMOD_IMG_INF;
			internal_img = 1;
			menu_exit();
			break;
		case MNU1_SLIDE_OFF:
			play_mode = PLYMOD_INF;
			internal_img = 0;
			menu_exit();
			break;
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
	static uint8_t sec_cnt=0;

	if(menu_on){
		menu_one_sec();
		sec_cnt=0;
		play_mode_old=-1;
		return;
	}


	if(play_mode_old!=play_mode){
		if(play_mode==PLYMOD_INF){
			LCD_Clear(BLUE);
			LCD_info_screen();
		}
		if(play_mode==PLYMOD_IMG){
			LCD_image_screen();
		}
		if(play_mode==PLYMOD_IMG_INF){
			LCD_image_screen();
			LCD_info_screen();
		}
		play_mode_old=play_mode;
	}

	//every IMG_TMOUT sec
	if((sec_cnt%IMG_TMOUT==0) && (sec_cnt>0)){
		if(play_mode==PLYMOD_INF){
			LCD_Clear(BLUE);
		}
		if((play_mode==PLYMOD_IMG) || (play_mode==PLYMOD_IMG_INF)){
			LCD_image_screen();
		}
		if((play_mode==PLYMOD_INF) || (play_mode==PLYMOD_IMG_INF)){
			LCD_info_screen();
		}
	}
	else{
	//every sec
		if((play_mode==PLYMOD_INF) || (play_mode==PLYMOD_IMG_INF)){
			LCD_update_time(BLUE);
		}
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

