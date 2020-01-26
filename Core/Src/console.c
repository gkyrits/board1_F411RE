/*
 * console.c
 *
 *  Created on: 24/12/2019
 *      Author: George
 */
#include <fatfs_utils.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "malloc.h"
#include "datetime.h"
#include "user_main.h"
#include "beeper.h"
#include "test_temp.h"

#define CRLN 	"\r\n"
#define PROMPT 	"cmd>"

#define HELP_CMD        "help"
#define MEM_CMD	        "memory"
#define DATE_CMD	    "get_date"
#define TIME_CMD	    "get_time"
#define SET_DATE_CMD    "set_date"
#define SET_TIME_CMD    "set_time"
#define EPOCH_CMD	    "get_epoch"
#define SET_EPOCH_CMD   "set_epoch"
#define SET_ALARM_CMD   "set_alarm"
#define ALARM_CMD   	"get_alarm"
#define BACKUP_CMD	    "backup"
#define GET_BACKUP_CMD  "get_backup"
#define BEEP_CMD        "beep"
#define BEEP_DEMO_CMD   "beep_demo"
#define OW_SCAN_CMD     "ow_scan"
#define LCD_BKLIT_CMD 	"lcd_bklit"
#define FSMOUNT_CMD     "fs_mount"
#define FSINFO_CMD      "fs_info"
#define FSFORMAT_CMD    "fs_format"
#define FSADDFILE_CMD   "fs_addfile"
#define FSSHOWFILE_CMD  "fs_showfile"
#define FSLIST_CMD      "fs_list"
#define PLAY_IMG_CMD    "slide_show"

#define HELP_CMD_DSCR    	"(command descriptions)"
#define MEM_CMD_DSCR	 	"(show memory)"
#define DATE_CMD_DSCR	 	"(show date-time)"
#define SET_DATE_CMD_DSCR   "[day] [date] [month] [year] (set_date)"
#define SET_TIME_CMD_DSCR   "[hour] [min] [sec] (set_time)"
#define EPOCH_CMD_DSCR	 	"(show epoch-time)"
#define SET_EPOCH_CMD_DSCR	"[epoch(hex)] (set epoch-time)"
#define SET_ALARM_CMD_DSCR  "[alrm:1/2] [date] [hour] [min] [sec] (set_alarm)"
#define ALARM_CMD_DSCR   	"(get alarms info)"
#define BACKUP_CMD_DSCR	   	"[text]	(save a backup text)"
#define GET_BACKUP_CMD_DSCR "(read the backup text)"
#define BEEP_CMD_DSCR       "[freq(Hz)] [time(ms)] (freq tone)"
#define BEEP_DEMO_CMD_DSCR  "(beep demo 100Hz-10000Hz / 100ms)"
#define OW_SCAN_CMD_DSCR    "(onewire scan test)"
#define LCD_BKLIT_CMD_DSCR	"[value(0-100)] (LCD BackLight value)"

#define FSMOUNT_CMD_DSCR     "(fs_mount)"
#define FSINFO_CMD_DSCR      "(fs_info)"
#define FSFORMAT_CMD_DSCR    "(fs_format)"
#define FSADDFILE_CMD_DSCR   "(fs_addfile)"
#define FSSHOWFILE_CMD_DSCR  "(fs_showfile)"
#define FSLIST_CMD_DSCR      "(fs_list)"

#define PLAY_IMG_CMD_DSCR    "[on/on_info/off] ([int]) (Play Slide Show)"


#define BUFF_LEN  160
static char cmd_line[BUFF_LEN];
static int cmd_idx=0;

static char prntBuff[BUFF_LEN];

static int  argc;
static char *argv[20];

int command_req=0;

//----------------------------------------------------------------------------------

static void cli_print_ch(const char ch){
	HAL_UART_Transmit(&huart2,(uint8_t*)&ch,1,HAL_MAX_DELAY);
}

static void cli_print(const char *data){
	int len = strlen(data);
	HAL_UART_Transmit(&huart2,(uint8_t*)data,len,HAL_MAX_DELAY);
}


void cli_printf(const char *frm, ...){
	va_list va;

    va_start(va,frm);
    vsnprintf(prntBuff, BUFF_LEN, frm, va);
    va_end(va);
    cli_print(prntBuff);
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
static void help_cmd(void){
	cli_printf(CRLN "%10s %s", HELP_CMD, HELP_CMD_DSCR);
	cli_printf(CRLN "%10s %s", MEM_CMD, MEM_CMD_DSCR);
	cli_printf(CRLN "%10s %s", DATE_CMD, DATE_CMD_DSCR);
	cli_printf(CRLN "%10s %s", TIME_CMD, DATE_CMD_DSCR);
	cli_printf(CRLN "%10s %s", SET_DATE_CMD, SET_DATE_CMD_DSCR);
	cli_printf(CRLN "%10s %s", SET_TIME_CMD, SET_TIME_CMD_DSCR);
	cli_printf(CRLN "%10s %s", EPOCH_CMD, EPOCH_CMD_DSCR);
	cli_printf(CRLN "%10s %s", SET_EPOCH_CMD, SET_EPOCH_CMD_DSCR);
	cli_printf(CRLN "%10s %s", SET_ALARM_CMD, SET_ALARM_CMD_DSCR);
	cli_printf(CRLN "%10s %s", ALARM_CMD, ALARM_CMD_DSCR);
	cli_printf(CRLN "%10s %s", BACKUP_CMD, BACKUP_CMD_DSCR);
	cli_printf(CRLN "%10s %s", GET_BACKUP_CMD, GET_BACKUP_CMD_DSCR);
	cli_printf(CRLN "%10s %s", BEEP_CMD, BEEP_CMD_DSCR);
	cli_printf(CRLN "%10s %s", BEEP_DEMO_CMD, BEEP_DEMO_CMD_DSCR);
	cli_printf(CRLN "%10s %s", OW_SCAN_CMD, OW_SCAN_CMD_DSCR);
	cli_printf(CRLN "%10s %s", LCD_BKLIT_CMD, LCD_BKLIT_CMD_DSCR);
	cli_printf(CRLN "%10s %s", FSMOUNT_CMD, FSMOUNT_CMD_DSCR);
	cli_printf(CRLN "%10s %s", FSINFO_CMD, FSINFO_CMD_DSCR);
	cli_printf(CRLN "%10s %s", FSFORMAT_CMD, FSFORMAT_CMD_DSCR);
	cli_printf(CRLN "%10s %s", FSADDFILE_CMD, FSADDFILE_CMD_DSCR);
	cli_printf(CRLN "%10s %s", FSSHOWFILE_CMD,FSSHOWFILE_CMD_DSCR);
	cli_printf(CRLN "%10s %s", FSLIST_CMD, FSLIST_CMD_DSCR);
	cli_printf(CRLN "%10s %s", PLAY_IMG_CMD, PLAY_IMG_CMD_DSCR);
	//...
	cli_print(CRLN);
}



//----------------------------------------------------------------------------------
extern int bss_size;
extern int heap_size;
extern int stack_size;
extern int free_mem;
extern int max_stack;
extern int min_freemem;
static void memory_cmd(void){
	printf("%s()\n",__FUNCTION__);
	cli_printf(CRLN"bss  size   : %d",bss_size);
	cli_printf(CRLN"heap size   : %d",heap_size);
	cli_printf(CRLN"cur stack   : %d",stack_size);
	cli_printf(CRLN"cur freeMem : %d",free_mem);
	cli_printf(CRLN"max stack   : %d",max_stack);
	cli_printf(CRLN"min freeMem : %d",min_freemem);
	cli_print(CRLN "---malloc info----");
	struct mallinfo mi = mallinfo();
	cli_printf(CRLN"Allocated   : %d", mi.arena);
	cli_printf(CRLN"Used        : %d", mi.uordblks);
	cli_printf(CRLN"Free        : %d", mi.fordblks);
	cli_printf(CRLN);
}

//----------------------------------------------------------------------------------
static void date_cmd(void){
	char *date = get_datetime_string();
	cli_printf(CRLN "%s",date);
	cli_print(CRLN PROMPT);
}

//----------------------------------------------------------------------------------
static void set_date_cmd(void){
	int ret;

	if(argc<5)
		goto help;

	ret=set_date_args(argv[1],argv[2],argv[3],argv[4]);
	if(ret==_OK)
		cli_print(CRLN "ok!");
	else
		cli_print(CRLN "fail set date");
	cli_print(CRLN PROMPT);

	return;
	help:
	cli_print(CRLN SET_DATE_CMD " " SET_DATE_CMD_DSCR);
}

//----------------------------------------------------------------------------------
static void set_time_cmd(void){
	int ret;

	if(argc<4)
		goto help;

	ret=set_time_args(argv[1],argv[2],argv[3]);
	if(ret==_OK)
		cli_print(CRLN "ok!");
	else
		cli_print(CRLN "fail set time");
	cli_print(CRLN PROMPT);

	return;
	help:
	cli_print(CRLN SET_TIME_CMD " " SET_TIME_CMD_DSCR);
}

//----------------------------------------------------------------------------------
static void epoch_cmd(void){
	U32 epoch = get_datetime_epoch();
	cli_printf(CRLN "epoch:%X",epoch);
	cli_print(CRLN PROMPT);
}

//----------------------------------------------------------------------------------
static void set_epoch_cmd(void){
	int val;

	if(argc<2)
		goto help;

	sscanf(argv[1],"%x",&val);
	set_datetime((U32)val);
	cli_printf(CRLN "set datetime to :%X",val);
	cli_print(CRLN PROMPT);

	return;
	help:
	cli_print(CRLN SET_EPOCH_CMD " " SET_EPOCH_CMD_DSCR);
}

//----------------------------------------------------------------------------------
static void set_alarm_cmd(void){
	int ret,alrm_id;

	if(argc<6)
		goto help;

	alrm_id=atoi(argv[1]);
	if((alrm_id<1) || (alrm_id>2))
		goto help;

	ret=set_alarm_args(alrm_id,argv[2],argv[3],argv[4],argv[5]);
	if(ret==_OK)
		cli_print(CRLN "ok!");
	else
		cli_print(CRLN "fail set alarm");
	cli_print(CRLN PROMPT);

	return;
	help:
	cli_print(CRLN SET_ALARM_CMD " " SET_ALARM_CMD_DSCR);
}

//----------------------------------------------------------------------------------
static void get_alarms_cmd(void){
	char alrm1[30],alrm2[30];
	int ret;
	ret=get_alarms(alrm1,alrm2);
	if(ret==_OK){
		cli_printf(CRLN "alrm1: %s",alrm1);
		cli_printf(CRLN "alrm2: %s",alrm2);
	}
	else
		cli_printf(CRLN "fail get alarms");
	cli_print(CRLN PROMPT);
}

//----------------------------------------------------------------------------------
#define BACKUP_SIZE 80
static void backup_cmd(void){
	int remain;
	char backup_txt[BACKUP_SIZE]="";
	if(argc<2)
		goto help;

	for(int ii=1; ii<argc; ii++){
		remain = BACKUP_SIZE - strlen(backup_txt);
		if(remain<2)
			break;
		if(strlen(argv[ii])>remain)
			argv[ii][remain-1]=0;
		strcat(backup_txt,argv[ii]);
		strcat(backup_txt," ");
	}
	write_backup_str(backup_txt);
	cli_printf(CRLN "save backup: [%s]",backup_txt);
	cli_printf(CRLN "%d of %d bytes",strlen(backup_txt),BACKUP_SIZE);
	cli_print(CRLN PROMPT);

	return;
	help:
	cli_print(CRLN BACKUP_CMD " " BACKUP_CMD_DSCR);
}

//----------------------------------------------------------------------------------
static void get_backup_cmd(void){
	char *text;

	text=read_backup_str();
	cli_printf(CRLN "read backup: [%s]",text);
	cli_printf(CRLN "%d of %d bytes",strlen(text),BACKUP_SIZE);
	cli_print(CRLN PROMPT);
}

//----------------------------------------------------------------------------------
static void beep_cmd(void){
	int freq,time=500;
	if(argc<2)
		goto help;

	freq=atoi(argv[1]);
	if(argc>2)
		time=atoi(argv[2]);
	if(freq<=0)
		goto help;
	if(time>10000)
		time=10000;
	cli_printf(CRLN "beep %dHz %dms ...",freq,time);
	beep(freq,time);
	cli_print(CRLN PROMPT);

	return;
	help:
	cli_print(CRLN BEEP_CMD " " BEEP_CMD_DSCR);
}

//----------------------------------------------------------------------------------
static void beep_demo_cmd(void){
	play_beeper_demo();
	cli_print(CRLN PROMPT);
}

//----------------------------------------------------------------------------------
static void lcd_backlit_cmd(void){
	int val;
	if(argc<2)
		goto help;

	val=atoi(argv[1]);
	if(val>=100)
		val=101;
	cli_printf(CRLN "LCD Backlight :%d",val);
	TIM_OC_InitTypeDef sConfig = {0};
	sConfig.OCMode = TIM_OCMODE_PWM1;
	sConfig.Pulse = val;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim11, &sConfig, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim11,TIM_CHANNEL_1);
	cli_print(CRLN PROMPT);

	return;
	help:
	cli_print(CRLN BEEP_CMD " " BEEP_CMD_DSCR);
}

//----------------------------------------------------------------------------------
extern uint8_t ow_sn[8],ow_data[9];
static void ow_scan_cmd(void){
	int ret;
	float temp;

	test_onewire();
	cli_print(CRLN "Serial: ");
	for(int ii=0; ii<8; ii++)
		cli_printf("%X,",ow_sn[ii]);
	cli_print(CRLN "Data: ");
	for(int ii=0; ii<9; ii++)
		cli_printf("%X,",ow_data[ii]);

	ret=get_temperature(&temp);
	if(ret!=0)
		cli_print(CRLN"error read temperature");
	else
		cli_printf(CRLN "Temperature: %.1f",temp);

	cli_print(CRLN);
}


//----------------------------------------------------------------------------------
static void fsinfo_cmd(void){
	char name[40];
	uint32_t tot_sect, fre_sect;
	int ret=get_fatfs_info(&tot_sect,&fre_sect,name);
	if(ret!=0){
		cli_printf(CRLN"error: %s",get_fatfs_error(ret));
	}
	else{
		cli_printf(CRLN" SD Name : [%s]",name);
	    /* Print the free space (assuming 512 bytes/sector) */
		cli_printf(CRLN"%10lu KB (%lu) total drive space."CRLN"%10lu KB (%lu) available.",tot_sect/2, tot_sect*512, fre_sect/2, fre_sect*512);
	}
	cli_print(CRLN);
}

//----------------------------------------------------------------------------------
static void fsmount_cmd(void){
	int en,ret;
	if(argc<2)
		goto help;

	en=atoi(argv[1]);
	ret=fatfs_mount(en);
	if(ret!=0)
		cli_printf(CRLN "mount(%d) error: %s",en,get_fatfs_error(ret));
	else
		cli_printf(CRLN "mount(%d) ok!",en);

	cli_print(CRLN);
	return;

	help:
	cli_print(CRLN FSMOUNT_CMD" [enable(0,1)]"CRLN);
}

//----------------------------------------------------------------------------------
static void fsformat_cmd(void){
	int ret;
	if(argc<2)
		goto help;

	ret=fatfs_format(argv[1]);
	if(ret!=0)
		cli_printf(CRLN "error: %s",get_fatfs_error(ret));
	else
		cli_printf(CRLN "ok!");

	cli_print(CRLN);
	return;

	help:
	cli_print(CRLN FSFORMAT_CMD" [name]"CRLN);
}

//----------------------------------------------------------------------------------
static void fsaddfile_cmd(void){
	int ret,lns;
	if(argc<3)
		goto help;

	lns=atoi(argv[2]);
	ret=fatfs_add_file(argv[1],lns);
	if(ret!=0)
		cli_printf(CRLN "error: %s",get_fatfs_error(ret));
	else
		cli_printf(CRLN "ok!");

	cli_print(CRLN);
	return;

	help:
	cli_print(CRLN FSADDFILE_CMD" [name] [lines]"CRLN);
}

//----------------------------------------------------------------------------------
static void fsshowfile_cmd(void){
	int ret;

	if(argc<2)
		goto help;

	cli_print(CRLN);
	ret=fatfs_read_file(argv[1],&cli_printf);
	if(ret!=0)
		cli_printf(CRLN "error: %s",get_fatfs_error(ret));
	else
		cli_printf(CRLN"...ok!");

	cli_print(CRLN);
	return;

	help:
	cli_print(CRLN FSLIST_CMD" [name]"CRLN);
}

//----------------------------------------------------------------------------------
static void fslist_cmd(void){
	int ret;
	char path[120];

	path[0]=0;
	cli_print(CRLN);
	ret=fatfs_list_files(path,&cli_printf);
	if(ret!=0)
		cli_printf(CRLN "error: %s",get_fatfs_error(ret));
	else
		cli_printf(CRLN"...ok!");
	cli_print(CRLN);
}


//----------------------------------------------------------------------------------
static void play_images_cmd(void){
	int err,num;

	if(argc<2)
		goto help;

	if(!strcmp(argv[1],"on"))
		play_mode    = PLYMOD_IMG;
	else if(!strcmp(argv[1],"on_info"))
		play_mode    = PLYMOD_IMG_INF;
	else if(!strcmp(argv[1],"off"))
		play_mode    = PLYMOD_INF;
	else
		goto help;

	if(play_mode == PLYMOD_INF){
		cli_printf(CRLN "slide show disabled");
		cli_print(CRLN);
		return;
	}

	internal_img = 0;
	if(argc>2){
		if(!strcmp(argv[2],"int"))
			internal_img=1;
	}

	if(internal_img){
		cli_printf(CRLN "read internal images");
		cli_print(CRLN);
		return;
	}

	num=read_image_list(&err);
	if(err!=NO_ERR){
		cli_printf(CRLN "error: %s",read_image_error(err));
		return;
	}
	cli_printf(CRLN "read %d images",num);
	if(num>0)
		internal_img=0;
	cli_print(CRLN);
	return;

	help:
	cli_printf(CRLN  PLAY_IMG_CMD " " PLAY_IMG_CMD_DSCR);
	cli_print(CRLN);
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

void parse_cmd(void){
	char *arg;

	//get arguments
	argc=0;
	arg = strtok(cmd_line," ");
	while(arg){
		argv[argc]=arg;
		argc++;
		arg = strtok(NULL," ");
	}

	//find command
	if(argc){
		if(!strcmp(argv[0],HELP_CMD))
			help_cmd();
		else if(!strcmp(argv[0],MEM_CMD))
			memory_cmd();
		else if(!strcmp(argv[0],DATE_CMD))
			date_cmd();
		else if(!strcmp(argv[0],TIME_CMD))
			date_cmd();
		else if(!strcmp(argv[0],SET_DATE_CMD))
			set_date_cmd();
		else if(!strcmp(argv[0],SET_TIME_CMD))
			set_time_cmd();
		else if(!strcmp(argv[0],EPOCH_CMD))
			epoch_cmd();
		else if(!strcmp(argv[0],SET_EPOCH_CMD))
			set_epoch_cmd();
		else if(!strcmp(argv[0],SET_ALARM_CMD))
			set_alarm_cmd();
		else if(!strcmp(argv[0],ALARM_CMD))
			get_alarms_cmd();
		else if(!strcmp(argv[0],BACKUP_CMD))
			backup_cmd();
		else if(!strcmp(argv[0],GET_BACKUP_CMD))
			get_backup_cmd();
		else if(!strcmp(argv[0],BEEP_CMD))
			beep_cmd();
		else if(!strcmp(argv[0],BEEP_DEMO_CMD))
			beep_demo_cmd();
		else if(!strcmp(argv[0],OW_SCAN_CMD))
			ow_scan_cmd();
		else if(!strcmp(argv[0],LCD_BKLIT_CMD))
			lcd_backlit_cmd();
		else if(!strcmp(argv[0],FSINFO_CMD))
			fsinfo_cmd();
		else if(!strcmp(argv[0],FSMOUNT_CMD))
			fsmount_cmd();
		else if(!strcmp(argv[0],FSFORMAT_CMD))
			fsformat_cmd();
		else if(!strcmp(argv[0],FSADDFILE_CMD))
			fsaddfile_cmd();
		else if(!strcmp(argv[0],FSSHOWFILE_CMD))
			fsshowfile_cmd();
		else if(!strcmp(argv[0],FSLIST_CMD))
			fslist_cmd();
		else if(!strcmp(argv[0],PLAY_IMG_CMD))
			play_images_cmd();
		//....
		else
			cli_print(CRLN "Unknown Command ?");
	}

	cmd_idx=0;
	command_req=0;
	cli_print(CRLN PROMPT);
}

//----------------------------------------------------------------------------------
void console_rx_char(const char ch){
	if(ch=='\r'){
		cmd_line[cmd_idx]=0;
		command_req=1;
	}
	else if(ch==127){ //back
		if(cmd_idx>0){
			cmd_idx--;
			cli_print_ch(ch);
		}
	}
	else{
		cmd_line[cmd_idx]=ch;
		cmd_idx++;
		cli_print_ch(ch);
	}
}

//----------------------------------------------------------------------------------
void init_console(){
	cli_print(CRLN HELLO_STR CRLN);
	cli_print(PROMPT);
}
