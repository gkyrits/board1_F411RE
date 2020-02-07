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

extern int write_record_line(const char *name ,char *line);


#endif /* INC_FATFS_UTILS_H_ */
