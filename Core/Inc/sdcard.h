/*
 * sdcard.h
 *
 *  Created on: 19 Ιαν 2020
 *      Author: George
 */

#ifndef INC_SDCARD_H_
#define INC_SDCARD_H_

typedef void (*cli_print_t)(const char *frm, ...);

extern int fatfs_mount(int enable);
extern int fatfs_format(const char* name);
extern int fatfs_add_file(const char *name ,int lines);
extern int fatfs_read_file(const char *name ,cli_print_t print);
extern int fatfs_list_files(char* path, cli_print_t print);

extern const char* get_fatfs_error(int err);
extern int get_fatfs_info(uint32_t *total_sect, uint32_t *free_sect, char *name);


#endif /* INC_SDCARD_H_ */
