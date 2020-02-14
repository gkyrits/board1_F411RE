/*
 * sdcard.h
 *
 *  Created on: 19 Ιαν 2020
 *      Author: George
 */

#ifndef INC_FATFS_UTILS_H_
#define INC_FATFS_UTILS_H_

#include <stdint.h>

typedef void (*cli_print_t)(const char *frm, ...);

extern int fatfs_mount(int enable);
extern int fatfs_format(const char* name);
extern int fatfs_add_file(const char *name ,int lines);
extern int fatfs_read_file(const char *name ,cli_print_t print);
extern int fatfs_list_files(char* path, cli_print_t print);

extern const char* get_fatfs_error(int err);
extern int get_fatfs_info(uint32_t *total_sect, uint32_t *free_sect, char *name);

//-------------------------------------------------------------------------
#define NO_ERR			  0
#define ERR_FS_MOUNT 	  1
#define ERR_MEMORY 		  2
#define ERR_FS_READ       3
#define ERR_FS_WRITE      4
#define ERR_NO_IMGS		  5

extern char* read_image_error(int err);
extern int read_image_list(int *err);
extern int LCD_load_next_image(void);

//-------------------------------------------------------------------------
#define DAY_REC_NUM  (24*60)

typedef struct time_rec{
	uint32_t time;
	int16_t  temp;
	int16_t  temp2;
	int8_t   humid;
}TIME_REC;

//24Hours records
typedef struct day_recs{
	TIME_REC rec[DAY_REC_NUM];
	uint16_t rec_num;
	uint32_t time;
}DAY_RECS;


extern int write_record_line(const char *name ,char *line);
extern DAY_RECS *read_record_block(const char *name , int *err);



#endif /* INC_FATFS_UTILS_H_ */
