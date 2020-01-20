/*
 * beeper.c
 *
 *  Created on: 19 Ιαν 2020
 *      Author: George
 */

#include "main.h"
#include "beeper.h"

//-------------------------------------------------------------------------
void play_beeper_demo(void){
	int div;
	HAL_TIM_PWM_Start(&htim10,TIM_CHANNEL_1);
	//up...
	for(int ii=100; ii<=10000; ii=ii+100){
		div= 1000000 / ii;
		htim10.Init.Prescaler=div;
		HAL_TIM_Base_Init(&htim10);
		HAL_Delay(50);
	}
	//down...
	for(int ii=10000; ii>=100; ii=ii-100){
		div= 1000000 / ii;
		htim10.Init.Prescaler=div;
		HAL_TIM_Base_Init(&htim10);
		HAL_Delay(50);
	}
	HAL_TIM_PWM_Stop(&htim10,TIM_CHANNEL_1);
}


//-------------------------------------------------------------------------
void beep(uint16_t freq, uint16_t timems){
	int div;
	div= 1000000 / freq;
	htim10.Init.Prescaler=div;
	HAL_TIM_Base_Init(&htim10);
	HAL_TIM_PWM_Start(&htim10,TIM_CHANNEL_1);
	HAL_Delay(timems);
	HAL_TIM_PWM_Stop(&htim10,TIM_CHANNEL_1);
}
