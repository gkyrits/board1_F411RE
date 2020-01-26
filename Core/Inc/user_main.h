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

extern void beep(uint16_t freq, uint16_t timems);

#define PLYMOD_INF 		0
#define PLYMOD_IMG 		1
#define PLYMOD_IMG_INF 	2
extern int play_mode;
extern int internal_img;

extern int alarm1_req,alarm2_req;



#endif /* INC_USER_MAIN_H_ */
