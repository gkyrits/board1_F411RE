/*
 * onewr_port.h
 *
 *  Created on: 3 Ιαν 2020
 *      Author: George
 */

#ifndef ONEWR_PORT_H_
#define ONEWR_PORT_H_

void Enable_micro_Timer(void);
void Delay_micro(uint32_t micros);
uint32_t Get_micros(void);


void GPIO_SetPinLow(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void GPIO_SetPinHigh(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

uint8_t GPIO_GetInputPinValue(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


#endif /* ONEWR_PORT_H_ */
