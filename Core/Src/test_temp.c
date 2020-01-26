/*
 * test_temp.c
 *
 *  Created on: 19 Ιαν 2020
 *      Author: George
 */

#include "stdio.h"
#include "string.h"
#include "main.h"

#include "onewire.h"
#include "ds1820.h"

static TM_OneWire_t onewr_port;
uint8_t ow_sn[8],ow_data[9];

//-------------------------------------------------------------------------
int get_temperature(float *temp){
	float value;
	int ret;
	if(TM_DS1820_Is(ow_sn)){
		ret=TM_DS1820_ParseData(ow_data,&value);
		if(ret!=_OK)
			return ret;
		*temp=value;
	}
	else
		return _ERR;
	return _OK;
}

//-------------------------------------------------------------------------
void test_onewire(void){
	//uint8_t data[9];
	uint8_t ow_stat,crc;
	uint16_t cnt;
	memset(ow_sn,0,sizeof(ow_sn));
	memset(ow_data,0,sizeof(ow_data));
	TM_OneWire_Init(&onewr_port,ONE_WIRE_GPIO_Port,ONE_WIRE_Pin);
	printf("Reset OneWire ...\n");
	ow_stat = TM_OneWire_Reset(&onewr_port);
	if(ow_stat==0){
		printf("Device found!\n");
		printf("Read Serial...\n");
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_READROM);
		for(int ii=0; ii<8; ii++)
			ow_sn[ii] = TM_OneWire_ReadByte(&onewr_port);
		printf("ROM: ");
		for(int ii=0; ii<8; ii++)
			printf("%X,",ow_sn[ii]);
		printf("\n");
		crc=TM_OneWire_CRC8(ow_sn,8);
		printf("crc=%X\n",crc);
		//..
		printf("Start Temp Conv...\n");
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_TEMP);
		printf("Wait...\n");
		ow_stat=0; cnt=0;
		while(!ow_stat){
			HAL_Delay(100);
			ow_stat = TM_OneWire_ReadBit(&onewr_port);
			cnt++;
			if(cnt>10){
				printf("time out temp!\n");
				TM_OneWire_Reset(&onewr_port);
				return;
			}
		}
		printf("Ready! cnt=%d\n",cnt);
		printf("Read pad: Reset...\n");
		ow_stat = TM_OneWire_Reset(&onewr_port);
		if(ow_stat==1){
			printf("fail detect on reset!\n");
			return;
		}
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_SKIPROM);
		TM_OneWire_WriteByte(&onewr_port,ONEWIRE_CMD_RSCRATCHPAD);
		for(int ii=0; ii<9; ii++)
			ow_data[ii] = TM_OneWire_ReadByte(&onewr_port);
		printf("PAD: ");
		for(int ii=0; ii<9; ii++)
			printf("%X,",ow_data[ii]);
		printf("\n");
		crc=TM_OneWire_CRC8(ow_data,9);
		printf("crc=%X\n",crc);
	}
	else
		printf("Presence NOT found!\n");
}

