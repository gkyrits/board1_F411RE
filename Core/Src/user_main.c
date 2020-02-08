/*
 * user_main.c
 *
 *  Created on: Dec 29, 2019
 *      Author: George
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <math.h>
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

int alrm1_req=0,alrm2_req=0;
int alrm1_en=0,alrm2_en=0;

int wakeup_req=0; //1min WakeUp Timer for get and save temperature!

int power_mode=PWRMOD_NORM;

static int menu_on=0;
static int menu_id;

int play_mode=PLYMOD_INF;
int internal_img=0;
static int play_mode_old=-1;

#define MENU_TIMEOUT 10
static int mnu_tmout_cnt=0;

static int graph_on=0;
static COLOR graph_col;

OPTIONS options;

//-------------------------------------------------------------------------
// UPDATE TEMPERATURE
//-------------------------------------------------------------------------
#define RECORD_FILE "records.txt"
static void write_temperature(float temp){
	char rec_line[80];
	int16_t temp16;
	uint32_t time;

	temp16 = temp*10;
	time = get_datetime_epoch();
	sprintf(rec_line,"%8X %03d",time,temp16);

	printf("rec:[%s]\n",rec_line);
	write_record_line(RECORD_FILE,rec_line);
}

//-------------------------------------------------------------------------
static void update_temperature(void){
	int ret;
	float temp;

	if(power_mode==PWRMOD_NORM)
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,GPIO_PIN_SET);
	//get temper
	for(int ii=0; ii<3; ii++){
		test_onewire();
		ret=get_temperature(&temp);
		if(ret==_OK)
			break;
	}
	if(power_mode==PWRMOD_NORM)
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,GPIO_PIN_RESET);
	if(ret==_OK)
		write_temperature(temp);
}

//-------------------------------------------------------------------------
// RECORDS GRAPH
//-------------------------------------------------------------------------
static int32_t find_screen_pos(int32_t min_val, int32_t max_val, int32_t val, int32_t min_pos, int32_t max_pos){
	int32_t val_mxmn,pos_mxmn,d1,value;
    double k, fd2,fvalue;

    val_mxmn = max_val - min_val;
    pos_mxmn = max_pos - min_pos;
    k = (double)pos_mxmn / (double)val_mxmn;

    d1 = val - min_val;
    fd2 = (double)d1 * k;

    fvalue = (double)min_pos + fd2;
    value = (int32_t)round(fvalue);

    return value;
}

//-------------------------------------------------------------------------
static void LCD_graph_screen(COLOR col, DAY_RECS *records){
	int16_t min_temp=900,max_temp=-900,cur_temp;
	uint32_t min_time,max_time;
	uint32_t min_temp_tm,max_temp_tm;
	uint16_t rec_num,xp,yp;
	date_time_t stime;
	char info[80];


	graph_on=1;
	graph_col = col;
	LCD_Clear(col);
	LCD_DrawRectangle (1, 1, LCD_WIDTH, LCD_HEIGHT-22, WHITE, DRAW_EMPTY , 1);

	rec_num=records->rec_num;
	//find limits
	for(int ii=0; ii<rec_num; ii++){
		if(records->rec[ii].temp<min_temp){
			min_temp = records->rec[ii].temp;
			min_temp_tm = records->rec[ii].time;
		}
		if(records->rec[ii].temp>max_temp){
			max_temp = records->rec[ii].temp;
			max_temp_tm = records->rec[ii].time;
		}
	}
	min_time=records->rec[0].time;
	max_time=records->rec[rec_num-1].time;
	cur_temp=records->rec[rec_num-1].temp;

	//print info
	epoch_to_date_time(&stime,min_temp_tm);
	sprintf(info,"min:%d %d:%d",min_temp,stime.hour,stime.minute);
	LCD_DisplayString(60,112,info,&Font8,graph_col,WHITE);
	epoch_to_date_time(&stime,max_temp_tm);
	sprintf(info,"max:%d %d:%d",max_temp,stime.hour,stime.minute);
	LCD_DisplayString(60,120,info,&Font8,graph_col,WHITE);
	sprintf(info,"%d",cur_temp);
	LCD_DisplayString(135,112,info,&Font12,graph_col,RED);

	//print graph
	for(int ii=0; ii<rec_num; ii++){
		xp = find_screen_pos(min_time,max_time,records->rec[ii].time,1,LCD_WIDTH-1);
		yp = find_screen_pos(min_temp-10,max_temp+10,records->rec[ii].temp,LCD_HEIGHT-22,1);
		LCD_SetPointlColor(xp,yp,YELLOW);
	}

}


//-------------------------------------------------------------------------
void LCD_records_graph(uint16_t col){
	DAY_RECS *records;
	int err;

	records = read_record_block(RECORD_FILE,&err);
	if(!records)
		return;

	LCD_graph_screen(col,records);

	free(records);
}


//-------------------------------------------------------------------------
// TEMP-DATE-TIME SCREEN
//-------------------------------------------------------------------------
static void LCD_update_time(COLOR col){
	char *date;
	date = get_time_string();
	if(graph_on){
		LCD_DisplayString(1,112,date,&Font12,graph_col,GREEN);
	}
	else{
		LCD_DisplayString(5,95,date,&Font20,col,GREEN);
	}
}

//-------------------------------------------------------------------------
static void LCD_info_screen(void){
	int ret;
	float temp;
	char temp_str[5];
	char *date_buf;
	//get temper
	ret=get_temperature(&temp);
	if(ret==_OK)
		sprintf(temp_str,"%04.1f",temp);
	else
		sprintf(temp_str,"--.-");
	//draw screen
	LCD_DisplayString(5,5,"Temperature",&Font12,LCD_BACKGROUND,YELLOW);
	LCD_DisplayString(5,20,temp_str,&Font24,LCD_BACKGROUND,RED);
	LCD_DisplayString(5,45,"Date",&Font12,LCD_BACKGROUND,YELLOW);
	date_buf = get_day_string();
	LCD_DisplayString(50,45,date_buf,&Font12,LCD_BACKGROUND,WHITE);
	date_buf = get_date_string();
	LCD_DisplayString(5,60,date_buf,&Font20,LCD_BACKGROUND,GREEN);
	LCD_DisplayString(5,80,"Time",&Font12,LCD_BACKGROUND,YELLOW);
	date_buf = get_time_string();
	LCD_DisplayString(5,95,date_buf,&Font20,BLUE,GREEN);
	//alarms
	if(alrm1_en)
		LCD_DisplayString(140,5,"A1",&Font12,LCD_BACKGROUND,YELLOW);
	if(alrm2_en)
		LCD_DisplayString(140,20,"A2",&Font12,LCD_BACKGROUND,YELLOW);
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
	update_options();
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
	if(menu_on){
		menu_action(menu_id,get_mnu_item(menu_id));
		return;
	}
	if(graph_on){
		graph_on=0;
		play_mode_old=-1;
		return;
	}
	LCD_records_graph(BLUE);
}


//-------------------------------------------------------------------------
//ONE_SEC
//-------------------------------------------------------------------------
static void one_sec_time_event(void){
	static uint8_t sec_cnt=0;
	static uint8_t reset_cnt=0;

	if(menu_on){
		menu_one_sec();
		sec_cnt=0;
		play_mode_old=-1;
		return;
	}

	if(graph_on){
		if(!reset_cnt){
			sec_cnt=0;
			reset_cnt=1;
		}
		if(sec_cnt>60){
			graph_on=0;
			reset_cnt=0;
			play_mode_old=-1;
		}
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
		reset_cnt=0;
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
static void apply_options(void){
	read_options(&options);
	play_mode = options.play_mode;
	internal_img = options.intrn_img;
	alrm1_en=options.alrm1_en;
	alrm2_en=options.alrm2_en;

	//init images
	if((play_mode==PLYMOD_IMG) || (play_mode==PLYMOD_IMG_INF)){
		if(!internal_img){
			int err;
			read_image_list(&err);
		}
	}

	//enable_alarms
	if(alrm1_en)
		enable_alarm(1,1);
	if(alrm2_en)
		enable_alarm(2,1);
}

//-------------------------------------------------------------------------
void update_options(void){
	options.play_mode = play_mode;
	options.intrn_img = internal_img;
	options.alrm1_en = alrm1_en;
	options.alrm2_en = alrm2_en;

	write_options(&options);
}

//-------------------------------------------------------------------------
//MAIN
//-------------------------------------------------------------------------
void main_init(void){
	printf(HELLO_STR "\n");
	init_datetime();
	apply_options();
	HAL_TIM_PWM_Start(&htim11,TIM_CHANNEL_1); //LCD PWM
	LCD_Init(D2U_L2R);
	LCD_Demo();
	init_console();
	update_temperature();
}


void main_loop(void){
	static int cnt = 0;
	if(power_mode==PWRMOD_NORM)
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	HAL_Delay(100);

	if(wakeup_req){ //1min WakeUp Timer for get and save temperature!
		//beep(500,50);
		update_temperature();
		if((!graph_on) && (!menu_on))
			play_mode_old=-1; //create screen update
		wakeup_req=0;
	}

	if(alrm1_req){
		beep(5000,50);
		alrm1_req=0;
	}
	if(alrm2_req){
		beep(4000,50);
		HAL_Delay(50);
		beep(4000,50);
		HAL_Delay(50);
		beep(4000,50);
		alrm2_req=0;
	}

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

