/*
 * user_main.h
 *
 *  Created on: Dec 29, 2019
 *      Author: George
 */

#ifndef INC_USER_MAIN_H_
#define INC_USER_MAIN_H_

#include "types.h"

#define USERSTR_SIZE	60 //of 80 bytes

typedef struct options{
	uint8_t alrm1_en,alrm2_en;
	uint8_t play_mode;
	uint8_t intrn_img;
}PACKED OPTIONS;  //program options

extern OPTIONS options;

extern void main_init(void);
extern void main_loop(void);
void update_options(void);

extern void button1_irq(void);
extern void button2_irq(void);

extern void beep(uint16_t freq, uint16_t timems);

extern int read_humidity(float *humid, float *temp);

extern void LCD_records_graph(uint16_t col);


#define PLYMOD_INF 		0
#define PLYMOD_IMG 		1
#define PLYMOD_IMG_INF 	2
extern int play_mode;
extern int internal_img;

extern int alrm1_req,alrm2_req;
extern int alrm1_en,alrm2_en;

extern int wakeup_req; //1min WakeUp Timer for get and save temperature!

#define PWRMOD_NORM		0
#define PWRMOD_LOW		1
#define PWRMOD_STOP		2
extern int power_mode;


#endif /* INC_USER_MAIN_H_ */
