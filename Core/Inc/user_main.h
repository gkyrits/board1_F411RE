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


#endif /* INC_USER_MAIN_H_ */
