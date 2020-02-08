/*
 * sdcard.c
 *
 *  Created on: 19 Ιαν 2020
 *      Author: George
 */
#include <fatfs_utils.h>
#include "stdio.h"
#include "string.h"
#include "fatfs.h"
#include "libjpeg.h"
#include "datetime.h"

static char workbuff[512];

#define MAX_IMAGES	   100
#define PATH_NAME_SIZE 30
typedef char IMAGE_LIST[MAX_IMAGES][PATH_NAME_SIZE];
static  IMAGE_LIST *image_list=0;
static int  img_num=0;
static int  img_idx=0;


extern void cli_printf(const char *frm, ...);

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

//-------------------------------------------------------------------------
static int fatfs_list_path_images(char* path, cli_print_t print, int add_list){
    FRESULT fr;     /* Return value */
    DIR dj;         /* Directory search object */
    FILINFO fno;    /* File information */
    char fullname[PATH_NAME_SIZE];

    fr = f_findfirst(&dj, &fno, path, "*.jpg");  /* Start to search for photo files */

    while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
    	snprintf(fullname,PATH_NAME_SIZE,"%s/%s",path, fno.fname);
    	if(print)
    		print("%s\n\r",fullname);                /* Display the object name */
    	if(add_list && image_list){
    		if(img_num==MAX_IMAGES)
    			break;
    		strncpy((*image_list)[img_num],fullname,PATH_NAME_SIZE);
    		img_num++;
    	}
        fr = f_findnext(&dj, &fno);               /* Search for next item */
    }

    f_closedir(&dj);
    return fr;
}


//-------------------------------------------------------------------------
int fatfs_list_images(cli_print_t print, int add_list){
    FRESULT res;
    DIR dir;
    FILINFO fno;

    //scan root dir
    fatfs_list_path_images("",print,add_list);

    //scan subdirs
    res = f_opendir(&dir, "");
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
            	fatfs_list_path_images(fno.fname,print,add_list);
            }
        }
        f_closedir(&dir);
    }

    return res;
}


//-------------------------------------------------------------------------
//  FS RECORDS ROYTINS
//-------------------------------------------------------------------------
int write_record_line(const char *name ,char *line){
	FIL MyFile;
	FRESULT ret;
	int rwbytes;

	//check if already mount
	ret=fatfs_mount(1);
	if(ret!=FR_OK){
		//error. try remount!
		fatfs_mount(0);
		ret=fatfs_mount(1);
		if(ret!=FR_OK)
			return ERR_FS_MOUNT;
	}

	ret = f_open(&MyFile, name, FA_OPEN_APPEND | FA_WRITE);
	if(ret!=FR_OK){
		printf("%s() fail f_open file[%s]\n",__FUNCTION__,name);
		return ERR_FS_WRITE;
	}

	rwbytes = f_printf(&MyFile, "%s\n", line);
	if(rwbytes<=0){
		f_close(&MyFile);
		printf("%s() fail f_printf\n",__FUNCTION__);
		return ERR_FS_WRITE;
	}
	printf("%s() write %d bytes\n",__FUNCTION__,rwbytes);

	ret = f_close(&MyFile);
	if(ret!=FR_OK)
		printf("%s() fail f_close\n",__FUNCTION__);

	return NO_ERR;
}


//-------------------------------------------------------------------------

#define DAY_SECS	 (24*60*60)

//-------------------------------------------------------------------------
static void parse_record_line(char *line ,DAY_RECS *records){
	int  argc;
	char *argv[3];
	char *arg;
	uint32_t start_time;
	uint32_t time;
	int16_t  temp;

	//split line to args
	argc=0;
	arg = strtok(line," ");
	while(arg){
		argv[argc]=arg;
		argc++;
		if(argc>=3)
			break;
		arg = strtok(NULL," ");
	}

	//check args
	if(argc<2)
		return;
	if(strlen(argv[0])!=8)
		return;
	if(strlen(argv[1])<3)
		return;
	sscanf(argv[0],"%X",&time);
	temp=atoi(argv[1]);

	//printf("rec[%d]  time=%X  temp=%d\n",records->rec_num,time,temp);
	start_time = records->time;

	if((temp==0) || (time==0))
		return;
	if((start_time-time)>DAY_SECS)
		return;
	if(records->rec_num>=DAY_REC_NUM)
		return;

	records->rec[records->rec_num].time=time;
	records->rec[records->rec_num].temp=temp;
	records->rec_num++;
}


//-------------------------------------------------------------------------
DAY_RECS *read_record_block(const char *name , int *err){
	FIL MyFile;
	FRESULT ret;
	TCHAR linebuff[160],*retbuff;
	DAY_RECS *records;
	uint32_t start_time;

	//check if already mount
	ret=fatfs_mount(1);
	if(ret!=FR_OK){
		//error. try remount!
		fatfs_mount(0);
		ret=fatfs_mount(1);
		if(ret!=FR_OK){
			*err=ERR_FS_MOUNT;
			return 0;
		}
	}

	ret = f_open(&MyFile, name, FA_OPEN_EXISTING | FA_READ);
	if(ret!=FR_OK){
		printf("%s() fail f_open file[%s]\n",__FUNCTION__,name);
		*err=ERR_FS_WRITE;
		return 0;
	}

	records=malloc(sizeof(DAY_RECS));
	if(!records){
		f_close(&MyFile);
		*err=ERR_MEMORY;
		return 0;
	}
	records->rec_num=0;

	start_time=get_datetime_epoch();
	records->time=start_time;
	for(;;){
		retbuff = f_gets(linebuff, sizeof(workbuff), &MyFile);
		if(retbuff==NULL)
			break;
		parse_record_line(linebuff,records);
		if(records->rec_num>=DAY_REC_NUM)
			break;
	}
	if(f_error(&MyFile))
		printf("%s() fail read!\n",__FUNCTION__);

	ret = f_close(&MyFile);
	if(ret!=FR_OK)
		printf("%s() fail f_close\n",__FUNCTION__);

	*err=NO_ERR;
	return records;
}



//-------------------------------------------------------------------------
//  FS IMAGE ROYTINS
//-------------------------------------------------------------------------
char* read_image_error(int err){
	switch(err){
	case NO_ERR: return "No Error";
	case ERR_FS_MOUNT: return "Fail mount SD";
	case ERR_MEMORY: return "Fail get mem";
	case ERR_FS_READ: return "Fail read SD";
	case ERR_NO_IMGS: return "No images";
	}
	return "Uknown Error";
}

//-------------------------------------------------------------------------
int read_image_list(int *err){
	int ret;

	*err=NO_ERR;
	img_idx=0;
	img_num=0;
	fatfs_mount(0);
	if(image_list)
		free(image_list);

	ret=fatfs_mount(1);
	if(ret){
		//play_mode=MODE_NULL;
		printf("%s() fail mount SD\n",__FUNCTION__);
		*err=ERR_FS_MOUNT;
		return 0;
	}

	if(!image_list)
		image_list=malloc(sizeof(IMAGE_LIST));
	if(!image_list){
		//play_mode=MODE_NULL;
		printf("%s() fail get mem\n",__FUNCTION__);
		*err=ERR_MEMORY;
		return 0;
	}

	ret=fatfs_list_images(0,1);
	if(ret){
		//play_mode=MODE_NULL;
		printf("%s() fail read SD\n",__FUNCTION__);
		free(image_list);
		image_list=0;
		*err=ERR_FS_READ;
		return 0;
	}

	//play_mode=MODE_FS_IMAGE;
	printf("%s() read %d images!\n",__FUNCTION__,img_num);
	return img_num;
}

//-------------------------------------------------------------------------
int LCD_load_next_image(void){
	int ret;

	if(!image_list)
		return ERR_NO_IMGS;

	if(img_idx>=img_num)
		img_idx=0;
	ret=load_image((*image_list)[img_idx]);
	if(ret)
		return ERR_FS_READ;
	img_idx++;

	return NO_ERR;
}

