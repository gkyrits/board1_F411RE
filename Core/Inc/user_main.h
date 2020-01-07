/*
 * user_main.h
 *
 *  Created on: Dec 29, 2019
 *      Author: George
 */

#ifndef INC_USER_MAIN_H_
#define INC_USER_MAIN_H_

extern void main_init(void);
extern void main_loop(void);

extern void button1_irq(void);
extern void button2_irq(void);

typedef void (*cli_print_t)(const char *frm, ...);

extern int fatfs_mount(int enable);
extern int fatfs_format(const char* name);
extern int fatfs_add_file(const char *name ,int lines);
extern int fatfs_read_file(const char *name ,cli_print_t print);
extern int fatfs_list_files(char* path, cli_print_t print);

extern const char* get_fatfs_error(int err);
extern int get_fatfs_info(uint32_t *total_sect, uint32_t *free_sect, char *name);

#endif /* INC_USER_MAIN_H_ */
