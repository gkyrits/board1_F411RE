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
typedef struct menu_item{
	char *descr;
	struct menu_item *next;
}MENU_ITEM;

typedef struct menu{
	int item_idx;
	MENU_ITEM *items;
}MENU;

#define MENU_NUM 3
MENU menu[MENU_NUM];

int menu_on=0;
int cur_menu_id;

int wait_mnu_sec=0;

//-------------------------------------------------------------------------
void add_mnu_option(int mnu_idx, char *item){
	MENU_ITEM **elem,*new_elem;

	elem = &menu[mnu_idx].items;
	while(*elem)
		elem=&(*elem)->next;

	new_elem=malloc(sizeof(MENU_ITEM));
	memset(new_elem,0,sizeof(MENU_ITEM));
	new_elem->descr=item;
	*elem=new_elem;
}

//-------------------------------------------------------------------------
void set_mnu_option(int mnu_idx, int item_idx){
	menu[mnu_idx].item_idx=item_idx;
}


//-------------------------------------------------------------------------
void show_menu(int mnu_id, COLOR col){
	MENU_ITEM *elem;
	int elem_num=0,elem_id,max_elem_len=0,elem_len;
	int x,y,ys;

	//LCD_Clear(col);
	//find elems info
	elem = menu[mnu_id].items;
	while(elem){
		elem_num++;
		elem_len=strlen(elem->descr);
		if(elem_len>max_elem_len)
			max_elem_len=elem_len;
		elem=elem->next;
	}
	//fix idx
	if(menu[mnu_id].item_idx>=elem_num)
		menu[mnu_id].item_idx=0;
	//estimate x,y;
	x=20;
	y=10;
	ys=16;
	//...
	//show items
	elem_id=0;
	elem = menu[mnu_id].items;
	while(elem){
		if(elem_id==menu[mnu_id].item_idx){
			LCD_DrawRectangle(x,y,x+120,y+ys-1,WHITE,DRAW_FULL,1);
			LCD_DisplayString(x,y,elem->descr,&Font16,LCD_BACKGROUND,col);
		}
		else{
			LCD_DrawRectangle(x,y,x+120,y+ys-1,col,DRAW_FULL,1);
			LCD_DisplayString(x,y,elem->descr,&Font16,LCD_BACKGROUND,WHITE);
		}
		elem=elem->next;
		elem_id++;
		y += ys;
	}
}

//-------------------------------------------------------------------------
void delete_menu(int mnu_id){
	MENU_ITEM *elem,*del_elem;

	elem = menu[mnu_id].items;
	while(elem){
		del_elem=elem;
		elem=elem->next;
		free(del_elem);
	}
	menu[mnu_id].items=0;
}

//-------------------------------------------------------------------------
void delete_menus(void){
	for(int ii=0; ii<MENU_NUM; ii++)
		delete_menu(ii);
}


//-------------------------------------------------------------------------
void create_menu1(){
	menu_on=1;
	cur_menu_id=0;
	add_mnu_option(cur_menu_id,"Item1");
	add_mnu_option(cur_menu_id,"Item2");
	add_mnu_option(cur_menu_id,"Exit");
	set_mnu_option(cur_menu_id,0);
	LCD_Clear(BLUE);
	show_menu(cur_menu_id,BLUE);
}


//-------------------------------------------------------------------------
void button_menu(void){
	wait_mnu_sec=0;
	if(!menu_on){
		create_menu1();
	}
	else{
		menu[cur_menu_id].item_idx++;
		show_menu(cur_menu_id,BLUE);
	}
}

//-------------------------------------------------------------------------
void menu_action(int mnu_id, int item_id){
	switch(mnu_id){
	case 0:
		if(item_id==2){
			delete_menus();
			menu_on=0;
		}
	}
}

//-------------------------------------------------------------------------
void button_select(void){
	if(!menu_on)
		return;
	menu_action(cur_menu_id,menu[cur_menu_id].item_idx);
}

//-------------------------------------------------------------------------
void menu_one_sec(void){
	wait_mnu_sec++;
	if(wait_mnu_sec>10){
		delete_menus();
		menu_on=0;
	}
}

//-------------------------------------------------------------------------
void one_sec_time_event(void){
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

