/*
 * onewr_port.c
 *
 *  Created on: 3 Ιαν 2020
 *      Author: George
 */

#include "main.h"

void Enable_micro_Timer(void){
    /* Enable TRC */
    CoreDebug->DEMCR &= ~0x01000000;
    CoreDebug->DEMCR |=  0x01000000;
    /* Enable counter */
    DWT->CTRL &= ~0x00000001;
    DWT->CTRL |=  0x00000001;
    /* Reset counter */
    DWT->CYCCNT = 0;
}

void Delay_micro(uint32_t micros){
	uint32_t start = DWT->CYCCNT;
	/* Go to number of cycles for system */
	micros *= (HAL_RCC_GetHCLKFreq() / 1000000);
	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
}


void GPIO_SetPinLow(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_RESET);
}

void GPIO_SetPinHigh(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_SET);
}

void GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

uint8_t GPIO_GetInputPinValue(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_PinState state = HAL_GPIO_ReadPin(GPIOx,GPIO_Pin);
	return state;
}

