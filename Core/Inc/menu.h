/*
 * menu.h
 *
 *  Created on: 20 Ιαν 2020
 *      Author: George
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

void add_mnu_option(int mnu_id, char *item);

void delete_menu(int mnu_id);
void delete_all_menu(void);

void set_mnu_item(int mnu_id, int item_idx);
void set_mnu_item_next(int mnu_id);
int get_mnu_item(int mnu_id);

void show_menu(int mnu_id, uint16_t col);

int show_info(char *text, uint16_t col);

#endif /* INC_MENU_H_ */
