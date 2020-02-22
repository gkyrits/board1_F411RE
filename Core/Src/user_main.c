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
#include "dht11.h"

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
static int grph_offs=0;

OPTIONS options;

//-------------------------------------------------------------------------
// UPDATE HUMIDITY (DHT11)
//-------------------------------------------------------------------------
DHT11_Dev dev;

static void init_dht11(void){
	DHT11_Init(&dev,DHT11_GPIO_Port,DHT11_Pin);
}

//-------------------------------------------------------------------------
int read_humidity(float *humid, float *temp){
	int ret;

	ret=DHT11_Read(&dev);
	if(ret!=DHT11_SUCCESS){
		printf("Error DHT11:%s\n",DHT11_error_str(ret));
		return _ERR;
	}

	*humid = dev.humidity;
	*temp  = dev.temparature;
	return _OK;
}

//-------------------------------------------------------------------------
static int get_humidity(float *humid, float *temp){

	*humid = dev.humidity;
	*temp  = dev.temparature;
	return dev.err;
}

//-------------------------------------------------------------------------
// UPDATE TEMPERATURE
//-------------------------------------------------------------------------
//#define RECORD_FILE "records.txt"
static int record_filename(char *fname){
	date_time_t date_time;
	int ret;

	ret=get_datetime_struct(&date_time);
	if(ret!=_OK)
		return ret;
	sprintf(fname,"rec%02d%02d.txt",date_time.year,date_time.month);
	return _OK;
}

//-------------------------------------------------------------------------
static void write_records(float temp, uint8_t humid, float temp2){
	char rec_line[80];
	char fname[20];
	int ret;
	int16_t temp16,temp16_2;
	uint32_t time;

	ret=record_filename(fname);
	if(ret!=_OK)
		return;

	temp16   = temp*10;
	temp16_2 = temp2*10;
	time = get_datetime_epoch();
	sprintf(rec_line,"%8X %03d %02d %03d",time,temp16,humid,temp16_2);

	printf("rec:[%s]\n",rec_line);
	write_record_line(fname,rec_line);
}

//-------------------------------------------------------------------------
static void update_sensors(void){
	int ret;
	float temp=0,temp2=0,humid=0;

	if(power_mode==PWRMOD_NORM)
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,GPIO_PIN_SET);

	//get temper
	for(int ii=0; ii<2; ii++){
		test_onewire();
		ret=get_temperature(&temp);
		if(ret==_OK)
			break;
	}
	//get humidity
	ret=read_humidity(&humid,&temp2);
	/*for(int ii=0; ii<2; ii++){
		ret=read_humidity(&humid,&temp2);
		if(ret==_OK)
			break;
		HAL_Delay(1000);
	}*/

	if(power_mode==PWRMOD_NORM)
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,GPIO_PIN_RESET);
	if(ret==_OK)
		write_records(temp,(uint8_t)humid,temp2);
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
	uint8_t min_humd=100,max_humd=0,cur_humd;
	uint16_t rec_num,xp,yp;
	date_time_t stime;
	char info[80];

	printf("%s()\n",__FUNCTION__);

	graph_on=1;
	graph_col = col;
	LCD_Clear(col);
	LCD_DrawRectangle (1, 1, LCD_WIDTH, LCD_HEIGHT-22, WHITE, DRAW_EMPTY , 1);

	rec_num=records->rec_num;
	//find limits
	for(int ii=0; ii<rec_num; ii++){
		//DS temp
		if(records->rec[ii].temp<min_temp){
			min_temp = records->rec[ii].temp;
			min_temp_tm = records->rec[ii].time;
		}
		if(records->rec[ii].temp>max_temp){
			max_temp = records->rec[ii].temp;
			max_temp_tm = records->rec[ii].time;
		}
		//DHT11
		if(records->rec[ii].humid>0){
			if(records->rec[ii].temp2<min_temp)
				min_temp = records->rec[ii].temp2;
			if(records->rec[ii].temp2>max_temp)
				max_temp = records->rec[ii].temp2;

			if(records->rec[ii].humid<min_humd)
				min_humd = records->rec[ii].humid;
			if(records->rec[ii].humid>max_humd)
				max_humd = records->rec[ii].humid;
		}
	}
	min_time=records->rec[0].time;
	max_time=records->rec[rec_num-1].time;
	cur_temp=records->rec[rec_num-1].temp;
	cur_humd=records->rec[rec_num-1].humid;

	//print info
	epoch_to_date_time(&stime,min_temp_tm);
	sprintf(info,"min:%d %d:%d",min_temp,stime.hour,stime.minute);
	LCD_DisplayString(60,110,info,&Font8,graph_col,WHITE);
	epoch_to_date_time(&stime,max_temp_tm);
	sprintf(info,"max:%d %d:%d",max_temp,stime.hour,stime.minute);
	LCD_DisplayString(60,120,info,&Font8,graph_col,WHITE);

	sprintf(info,"%d-%d",min_humd,max_humd);
	LCD_DisplayString(130,120,info,&Font8,graph_col,CYAN);

	sprintf(info,"%d %d",cur_temp,cur_humd);
	LCD_DisplayString(130,110,info,&Font8,graph_col,RED);


	for(int ii=0; ii<rec_num; ii++){
		xp = find_screen_pos(min_time,max_time,records->rec[ii].time,1,LCD_WIDTH-2);

		//print graph DHT11
		if(records->rec[ii].humid>0){
			//print graph DHT11 temp
			/*yp = find_screen_pos(min_temp-10,max_temp+10,records->rec[ii].temp2,LCD_HEIGHT-22,1);
			LCD_SetPointlColor(xp,yp,RED);*/
			//print graph DHT11 humid
			yp = find_screen_pos(min_humd-10,max_humd+10,records->rec[ii].humid,LCD_HEIGHT-22,1);
			LCD_SetPointlColor(xp,yp,CYAN);
		}

		//print graph DS temp
		yp = find_screen_pos(min_temp-10,max_temp+10,records->rec[ii].temp,LCD_HEIGHT-22,1);
		LCD_SetPointlColor(xp,yp,YELLOW);

	}

}


//-------------------------------------------------------------------------
void LCD_records_graph(uint16_t col){
	char fname[20];
	DAY_RECS *records;
	int ret,err;

	grph_offs=0;
	ret=record_filename(fname);
	if(ret!=_OK)
		return;

	records = read_record_block(fname,&err,0);
	if(!records)
		return;

	LCD_graph_screen(col,records);

	free(records);
}

//-------------------------------------------------------------------------
void LCD_records_graph_next(uint16_t col){
	char fname[20];
	DAY_RECS *records;
	int ret,err;

	ret=record_filename(fname);
	if(ret!=_OK)
		return;

	grph_offs += DAY_SECS;
	printf("%s() grph_offs=%d\n",__FUNCTION__,grph_offs);
	records = read_record_block(fname,&err,grph_offs);
	if(!records){
		LCD_records_graph(col);
		return;
	}
	if(records->rec_num==0){
		free(records);
		LCD_records_graph(col);
		return;
	}

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
		if(!grph_offs)
			LCD_DisplayString(1,114,date,&Font12,graph_col,GREEN);
		else{
			date = get_offs_date_string(grph_offs);
			LCD_DisplayString(1,114,date,&Font12,graph_col,GREEN);
		}
	}
	else{
		LCD_DisplayString(5,95,date,&Font20,col,GREEN);
	}
}

//-------------------------------------------------------------------------
static void LCD_info_screen(void){
	int ret;
	float temp,humid,temp2;
	char temp_str[5],temp2_str[5],humid_str[3];
	char *date_buf;
	//get temper
	ret=get_temperature(&temp);
	if(ret==_OK)
		sprintf(temp_str,"%.1f",temp);
	else
		sprintf(temp_str,"--.-");
	//get humidity
	ret=get_humidity(&humid,&temp2);
	sprintf(temp2_str,"%.1f",temp2);
	sprintf(humid_str,"%d",(uint8_t)humid);
	if(ret==DHT11_ERROR_TIMEOUT)
		strcat(temp2_str," t");
	else if(ret==DHT11_ERROR_CHECKSUM)
		strcat(temp2_str," s");

	//draw screen
	LCD_DisplayString(5,5,"Temperature",&Font12,LCD_BACKGROUND,YELLOW);
	LCD_DisplayString(5,22,temp_str,&Font20,LCD_BACKGROUND,RED);
	LCD_DisplayString(90,22,humid_str,&Font20,LCD_BACKGROUND,CYAN);
	LCD_DisplayString(90,5,temp2_str,&Font12,LCD_BACKGROUND,CYAN);
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
		LCD_DisplayString(145,5,"A1",&Font12,LCD_BACKGROUND,YELLOW);
	if(alrm2_en)
		LCD_DisplayString(145,20,"A2",&Font12,LCD_BACKGROUND,YELLOW);
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

//----------------------------------------------------------------------------------
// POWER MODE
//----------------------------------------------------------------------------------

void set_LCD_backlight(int value){
	TIM_OC_InitTypeDef sConfig = {0};
	sConfig.OCMode = TIM_OCMODE_PWM1;
	sConfig.Pulse = value;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim11, &sConfig, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim11,TIM_CHANNEL_1);
}

//----------------------------------------------------------------------------------
void apply_power_mode(void){
	switch(power_mode){
	case PWRMOD_NORM:
		set_LCD_backlight(101);
		break;
	case PWRMOD_LOW:
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,GPIO_PIN_RESET);
		set_LCD_backlight(10);
		break;
	case PWRMOD_STOP:
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,GPIO_PIN_RESET);
		set_LCD_backlight(0);
		HAL_PWREx_EnableFlashPowerDown();
		LCD_Sleep(1);
		HAL_Delay(100);
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);
		SystemClock_Config();
		HAL_PWREx_DisableFlashPowerDown();
		LCD_Sleep(0);
		power_mode=PWRMOD_LOW; //...
		if(power_mode==PWRMOD_LOW)
			set_LCD_backlight(10);
		else
			set_LCD_backlight(101);
		break;
	}
}


//-------------------------------------------------------------------------
//MENUS
//-------------------------------------------------------------------------
#define MAIN_MENU			0 //MNU1
#define POWER_MENU			1 //MNU2

//MAIN_MENU
#define MNU1_SLIDE_SD		0
#define MNU1_SLIDE_SD_INF	1
#define MNU1_SLIDE_INT_INF	2
#define MNU1_SLIDE_OFF		3
#define MNU1_POWER_MODE		4
#define MNU1_TEST_BEEP		5
#define MNU1_EXIT			6

//POWER_MENU
#define MNU2_PWR_NORM		0
#define MNU2_PWR_LOW		1
#define MNU2_PWR_STOP		2
#define MNU2_EXIT			3

//-------------------------------------------------------------------------
static void create_main_menu(){
	menu_id=MAIN_MENU;
	add_mnu_option(menu_id,"Slide sd");
	add_mnu_option(menu_id,"Slide sd/info");
	add_mnu_option(menu_id,"Slide int/info");
	add_mnu_option(menu_id,"Slide off");
	add_mnu_option(menu_id,"Power mode");
	add_mnu_option(menu_id,"Beeper Test");
	add_mnu_option(menu_id,"Exit");
	set_mnu_item(menu_id,MNU1_SLIDE_SD);
	LCD_Clear(BLUE);
	show_menu(menu_id,BLUE);
}

//-------------------------------------------------------------------------
static void create_power_menu(){
	menu_id=POWER_MENU;
	add_mnu_option(menu_id,"Normal");
	add_mnu_option(menu_id,"Low Power");
	add_mnu_option(menu_id,"Stop Mode");
	add_mnu_option(menu_id,"Exit");
	set_mnu_item(menu_id,MNU2_PWR_NORM);
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
	case MAIN_MENU:
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
		case MNU1_POWER_MODE:
			create_power_menu();
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
		break; //MAIN_MENU
	case POWER_MENU:
		switch(item_id){
		case MNU2_PWR_NORM:
			power_mode=PWRMOD_NORM;
			menu_exit();
			apply_power_mode();
			break;
		case MNU2_PWR_LOW:
			power_mode=PWRMOD_LOW;
			menu_exit();
			apply_power_mode();
			break;
		case MNU2_PWR_STOP:
			power_mode=PWRMOD_STOP;
			menu_exit();
			apply_power_mode();
			break;
		case MNU2_EXIT:
			delete_menu(POWER_MENU);
			menu_id=MAIN_MENU;
			LCD_Clear(BLUE);
			show_menu(menu_id,BLUE);
			break;
		}
		break; //POWER_MENU
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
	if(graph_on){
		LCD_records_graph_next(BLUE);
		return;
	}
	mnu_tmout_cnt=0;
	if(!menu_on){
		create_main_menu();
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
	power_mode = options.pwr_mod;

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

	//power mode
	apply_power_mode();
}

//-------------------------------------------------------------------------
void update_options(void){
	options.play_mode = play_mode;
	options.intrn_img = internal_img;
	options.alrm1_en = alrm1_en;
	options.alrm2_en = alrm2_en;
	options.pwr_mod  = power_mode;

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
	HAL_Delay(100);
	init_console();
	init_dht11();
	update_sensors();
}


void main_loop(void){
	static int cnt = 0;
	if(power_mode==PWRMOD_NORM)
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	HAL_Delay(100);

	if(wakeup_req){ //1min WakeUp Timer for get and save temperature!
		//beep(500,50);
		update_sensors();
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

