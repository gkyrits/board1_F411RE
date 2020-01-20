/*
 * menu.c
 *
 *  Created on: 20 Ιαν 2020
 *      Author: George
 */

#include "stdlib.h"
#include "string.h"
#include "LCD.h"
#include "menu.h"


//-------------------------------------------------------------------------
typedef struct menu_item{
	char *descr;
	struct menu_item *next;
}MENU_ITEM;

typedef struct menu{
	int item_idx;
	MENU_ITEM *items;
}MENU;

#define MENU_NUM 3
static MENU menu[MENU_NUM];


//-------------------------------------------------------------------------
void add_mnu_option(int mnu_id, char *item){
	MENU_ITEM **elem,*new_elem;

	if(mnu_id>MENU_NUM)
		return;
	elem = &menu[mnu_id].items;
	while(*elem)
		elem=&(*elem)->next;

	new_elem=malloc(sizeof(MENU_ITEM));
	memset(new_elem,0,sizeof(MENU_ITEM));
	new_elem->descr=item;
	*elem=new_elem;
}

//-------------------------------------------------------------------------
void delete_menu(int mnu_id){
	MENU_ITEM *elem,*del_elem;

	if(mnu_id>MENU_NUM)
		return;
	elem = menu[mnu_id].items;
	while(elem){
		del_elem=elem;
		elem=elem->next;
		free(del_elem);
	}
	menu[mnu_id].items=0;
}

//-------------------------------------------------------------------------
void delete_all_menu(void){
	for(int ii=0; ii<MENU_NUM; ii++)
		delete_menu(ii);
}

//-------------------------------------------------------------------------
void set_mnu_item(int mnu_id, int item_idx){
	if(mnu_id>MENU_NUM)
		return;
	menu[mnu_id].item_idx=item_idx;
}


//-------------------------------------------------------------------------
void set_mnu_item_next(int mnu_id){
	if(mnu_id>MENU_NUM)
		return;
	menu[mnu_id].item_idx++;
}

//-------------------------------------------------------------------------
int get_mnu_item(int mnu_id){
	if(mnu_id>MENU_NUM)
		return -1;
	return menu[mnu_id].item_idx;
}

//-------------------------------------------------------------------------
int show_info(char *text, COLOR col){
	int x,y,ys;
	//estimate x,y;
	x=20;
	y=35;
	ys=16;
	LCD_DrawRectangle(x,y,x+120,y+(ys*3)-1,col,DRAW_FULL,1);
	LCD_DisplayString(x,y+ys,text,&Font16,LCD_BACKGROUND,WHITE);
}


//-------------------------------------------------------------------------
void show_menu(int mnu_id, COLOR col){
	MENU_ITEM *elem;
	int elem_num=0,elem_id,max_elem_len=0,elem_len;
	int x,y,ys;

	if(mnu_id>MENU_NUM)
		return;
	//find elems info
	elem = menu[mnu_id].items;
	while(elem){
		elem_num++;
		elem_len=strlen(elem->descr);
		if(elem_len>max_elem_len)
			max_elem_len=elem_len;
		elem=elem->next;
	}
	//fix idx
	if(menu[mnu_id].item_idx>=elem_num)
		menu[mnu_id].item_idx=0;
	//estimate x,y;
	x=20;
	y=10;
	ys=16;
	//...
	//show items
	elem_id=0;
	elem = menu[mnu_id].items;
	while(elem){
		if(elem_id==menu[mnu_id].item_idx){
			LCD_DrawRectangle(x,y,x+120,y+ys-1,WHITE,DRAW_FULL,1);
			LCD_DisplayString(x,y,elem->descr,&Font16,LCD_BACKGROUND,col);
		}
		else{
			LCD_DrawRectangle(x,y,x+120,y+ys-1,col,DRAW_FULL,1);
			LCD_DisplayString(x,y,elem->descr,&Font16,LCD_BACKGROUND,WHITE);
		}
		elem=elem->next;
		elem_id++;
		y += ys;
	}
}




