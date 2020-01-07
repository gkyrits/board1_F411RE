/*
 * console.h
 *
 *  Created on: 28 Οκτ 2019
 *      Author: George
 */

#ifndef INC_CONSOLE_H_
#define INC_CONSOLE_H_

extern int command_req;

extern void init_console();

extern void parse_cmd(void);

extern void console_rx_char(const char ch);

#endif /* INC_CONSOLE_H_ */
