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

#include "user_main.h"
#include "onewire.h"
#include "LCD.h"
#include "fatfs.h"

static int cnt=0;

static int but1_req=0,but2_req=0;

static TM_OneWire_t onewr_port;
uint8_t ow_sn[8],ow_data[9];

static char workbuff[512];

extern void cli_printf(const char *frm, ...);
void test_onewire(void){
	//uint8_t data[9];
	uint8_t ow_stat,crc;
	uint16_t cnt;
	memset(ow_sn,0,sizeof(ow_sn));
	memset(ow_data,0,sizeof(ow_data));
	TM_OneWire_Init(&onewr_port,ONE_WIRE_GPIO_Port,ONE_WIRE_Pin);
	printf("Reset OneWire ...\n");
	ow_stat = TM_OneWire_Reset(&onewr_port);
	if(ow_stat==0){
		printf("Device found!\n");
		printf("Read Serial...\n");
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_READROM);
		for(int ii=0; ii<8; ii++)
			ow_sn[ii] = TM_OneWire_ReadByte(&onewr_port);
		printf("ROM: ");
		for(int ii=0; ii<8; ii++)
			printf("%X,",ow_sn[ii]);
		printf("\n");
		crc=TM_OneWire_CRC8(ow_sn,8);
		printf("crc=%X\n",crc);
		//..
		printf("Start Temp Conv...\n");
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_TEMP);
		printf("Wait...\n");
		ow_stat=0; cnt=0;
		while(!ow_stat){
			HAL_Delay(100);
			ow_stat = TM_OneWire_ReadBit(&onewr_port);
			cnt++;
			if(cnt>10){
				printf("time out temp!\n");
				TM_OneWire_Reset(&onewr_port);
				return;
			}
		}
		printf("Ready! cnt=%d\n",cnt);
		printf("Read pad: Reset...\n");
		ow_stat = TM_OneWire_Reset(&onewr_port);
		if(ow_stat==1){
			printf("fail detect on reset!\n");
			return;
		}
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_SKIPROM);
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_RSCRATCHPAD);
		for(int ii=0; ii<9; ii++)
			ow_data[ii] = TM_OneWire_ReadByte(&onewr_port);
		printf("PAD: ");
		for(int ii=0; ii<9; ii++)
			printf("%X,",ow_data[ii]);
		printf("\n");
		crc=TM_OneWire_CRC8(ow_data,9);
		printf("crc=%X\n",crc);
	}
	else
		printf("Presence NOT found!\n");
}

//-------------------------------------------------------------------------
const char* get_fatfs_error(int err){
	switch(err){
	case FR_OK:						return "0.Succeeded";
	case FR_DISK_ERR:				return "1.A hard error occurred in the low level disk I/O layer";
	case FR_INT_ERR:				return "2.Assertion failed";
	case FR_NOT_READY:				return "3.The physical drive cannot work";
	case FR_NO_FILE:				return "4.Could not find the file";
	case FR_NO_PATH:				return "5.Could not find the path";
	case FR_INVALID_NAME:			return "6.The path name format is invalid";
	case FR_DENIED:					return "7.Access denied due to prohibited access or directory full";
	case FR_EXIST:					return "8.Access denied due to prohibited access";
	case FR_INVALID_OBJECT:			return "9.The file/directory object is invalid";
	case FR_WRITE_PROTECTED:		return "10.The physical drive is write protected";
	case FR_INVALID_DRIVE:			return "11.The logical drive number is invalid";
	case FR_NOT_ENABLED:			return "12.The volume has no work area";
	case FR_NO_FILESYSTEM:			return "13.There is no valid FAT volume";
	case FR_MKFS_ABORTED:			return "14.The f_mkfs() aborted due to any problem";
	case FR_TIMEOUT:				return "15.Could not get a grant to access the volume within defined period";
	case FR_LOCKED:					return "16.The operation is rejected according to the file sharing policy";
	case FR_NOT_ENOUGH_CORE:		return "17.LFN working buffer could not be allocated";
	case FR_TOO_MANY_OPEN_FILES:	return "18.Number of open files > _FS_LOCK";
	case FR_INVALID_PARAMETER:		return "19.Given parameter is invalid";
	}
	return "Unknown";
}


//-------------------------------------------------------------------------
int fatfs_mount(int enable){
	FRESULT ret;

	printf("%s(%d)\n",__FUNCTION__,enable);
	if(enable){
		MX_FATFS_Init();
		ret = f_mount(&USERFatFS, (TCHAR const*)USERPath, 1);
	}
	else{
		ret = f_mount(NULL, (TCHAR const*)USERPath, 1);
		HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
		MX_FATFS_DeInit();
	}

	if(ret!=FR_OK)
		printf("fail f_mount\n");

	return ret;
}

//-------------------------------------------------------------------------
int fatfs_format(const char* name){
	FRESULT ret;

	printf("%s(%s)\n",__FUNCTION__,name);

	ret = f_mkfs((TCHAR const*)USERPath,FM_FAT,0,workbuff,sizeof(workbuff));
	if(ret!=FR_OK){
		printf("fail f_mkfs\n");
		return ret;
	}

	ret = f_setlabel((TCHAR const*)name);
	if(ret!=FR_OK){
		printf("fail f_setlabel\n");
		return ret;
	}

	return ret;
}


//-------------------------------------------------------------------------
int get_fatfs_info(uint32_t *total_sect, uint32_t *free_sect, char *name){
	FATFS *fs;
	FRESULT ret;
    DWORD fre_clust, fre_sect, tot_sect, sn;

    /* Get volume information and free clusters of drive 1 */
    ret = f_getfree((TCHAR const*)USERPath, &fre_clust, &fs);
	if(ret!=FR_OK){
		printf("fail f_getfree:%d\n",ret);
		return ret;
	}
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    printf("tot_sect:%ld  fre_sect:%ld\n",tot_sect,fre_sect);
    *total_sect = (uint32_t)tot_sect;
    *free_sect = (uint32_t)fre_sect;

    if(name!=NULL){
    	ret= f_getlabel((TCHAR const*)USERPath, (TCHAR*)name, &sn);
    	if(ret!=FR_OK){
    		printf("fail f_getlabel:%d\n",ret);
    		return ret;
    	}
    	printf("name:%s  sn:%lu\n",name,sn);
    }

    return ret;
}

//-------------------------------------------------------------------------
int fatfs_add_file(const char *name ,int lines){
	FIL MyFile;
	FRESULT ret;
	UINT rwbytes,sumbytes=0,partbytes=0;
	char linebuff[80];

	ret = f_open(&MyFile, name, FA_CREATE_ALWAYS | FA_WRITE);
	if(ret!=FR_OK){
		printf("fail f_open file[%s]\n",name);
		return ret;
	}

	sprintf(linebuff,"Test file with %d lines!\n",lines);
	ret = f_write(&MyFile, linebuff, strlen(linebuff), &rwbytes);
	if(ret!=FR_OK){
		f_close(&MyFile);
		printf("fail f_write\n");
		return ret;
	}
	sumbytes += rwbytes;
	partbytes += rwbytes;
	for(int ii=0; ii<lines; ii++){
		sprintf(linebuff,"Line %d\n",ii+1);
		ret = f_write(&MyFile, linebuff, strlen(linebuff), &rwbytes);
		if(ret!=FR_OK){
			f_close(&MyFile);
			printf("fail f_write after %d bytes!\n",sumbytes);
			return ret;
		}
		sumbytes += rwbytes;
		partbytes += rwbytes;
		if(partbytes > 1024){ //every 1K delay
			partbytes=0;
			HAL_Delay(10);
			//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		}
	}
	printf("write %d bytes on file[%s]\n",sumbytes,name);

	ret = f_close(&MyFile);
	if(ret!=FR_OK){
		printf("fail f_close\n");
		return ret;
	}
	return ret;
}

//-------------------------------------------------------------------------
int fatfs_read_file(const char *name ,cli_print_t print){
	FIL MyFile;
	FRESULT ret;
	TCHAR linebuff[160],*retbuff;
	//int line_cnt=0;

	ret = f_open(&MyFile, name, FA_OPEN_EXISTING | FA_READ);
	if(ret!=FR_OK){
		printf("fail f_open file[%s]\n",name);
		return ret;
	}

	for(;;){
		retbuff = f_gets(linebuff, sizeof(workbuff), &MyFile);
		if(retbuff!=NULL){
			if(print!=NULL)
				print("%s\r",linebuff);
			/*line_cnt++;
			if(!(line_cnt % 100))
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);*/
		}
		else
			break;
	}
	if(f_error(&MyFile))
		printf("fail read!\n");

	ret = f_close(&MyFile);
	if(ret!=FR_OK){
		printf("fail f_close\n");
		return ret;
	}

	return ret;
}


//-------------------------------------------------------------------------
int fatfs_list_files(char* path, cli_print_t print){
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = fatfs_list_files(path,print);        /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
            	if(print!=NULL)
            		print("%s/%s\n\r", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}


void main_init(void){
	printf(HELLO_STR "\n");
	init_datetime();
	HAL_TIM_PWM_Start(&htim11,TIM_CHANNEL_1);
	LCD_Init(D2U_L2R);
	LCD_Show();
	HAL_Delay(100);
	init_console();
}


void main_loop(void){
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	HAL_Delay(200);
	cnt++;

	if(cnt==20){
		LCD_DisplayImage(0,0,&Image1);
	}
	else if(cnt==40){
		LCD_DisplayImage(0,0,&Image2);
	}
	else if(cnt==60){
		LCD_Clear(WHITE);
		LCD_Show();
	}
	if(cnt>80)
		cnt=0;

	if(command_req)
		parse_cmd();

	if(but1_req){
		int div= 1000000 / 3000; //3KHz
		htim10.Init.Prescaler=div;
		HAL_TIM_Base_Init(&htim10);
		HAL_TIM_PWM_Start(&htim10,TIM_CHANNEL_1);
		HAL_Delay(100);
		HAL_TIM_PWM_Stop(&htim10,TIM_CHANNEL_1);
		but1_req=0;
	}

	if(but2_req){
		int div= 1000000 / 1000; //1KHz
		htim10.Init.Prescaler=div;
		HAL_TIM_Base_Init(&htim10);
		HAL_TIM_PWM_Start(&htim10,TIM_CHANNEL_1);
		HAL_Delay(100);
		HAL_TIM_PWM_Stop(&htim10,TIM_CHANNEL_1);
		HAL_Delay(100);
		HAL_TIM_PWM_Start(&htim10,TIM_CHANNEL_1);
		HAL_Delay(100);
		HAL_TIM_PWM_Stop(&htim10,TIM_CHANNEL_1);
		but2_req=0;
	}
}


void button1_irq(void){
	but1_req=1;
}

void button2_irq(void){
	but2_req=1;
}

