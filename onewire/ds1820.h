/*
 * ds1820.h
 *
 *  Created on: 18 Ιαν 2020
 *      Author: George
 */

#ifndef DS1820_H_
#define DS1820_H_

#define _OK  0
#define _ERR 1

extern int TM_DS1820_Is(uint8_t *ROM);

extern int TM_DS1820_ParseData(uint8_t *data, float *temper_val);

#endif /* DS1820_H_ */
