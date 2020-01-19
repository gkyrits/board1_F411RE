/*
 * ds1820.c
 *
 *  Created on: 18 Ιαν 2020
 *      Author: George
 */

#include "stdio.h"
#include "onewire.h"
#include "ds1820.h"

#define DS1820_FAMILY_CODE 0x10

#define DS1820_DECIMAL_STEPS 0.5

int TM_DS1820_Is(uint8_t *ROM) {
	/* Checks if first byte is equal to DS1820's family code */
	if (*ROM == DS1820_FAMILY_CODE) {
		return 1;
	}
	return 0;
}


int TM_DS1820_ParseData(uint8_t *data, float *temper_val) {
	int16_t temperature,temp_read;
	float cnt_remain, cnt_perc;
	float temper,hres_temper;
	uint8_t crc;

	/* Calculate CRC */
	crc = TM_OneWire_CRC8(data, 8);

	/* Check if CRC is ok */
	if (crc != data[8]) {
		/* CRC invalid */
		return _ERR;
	}

	/* First two bytes of scratchpad are temperature values */
	temperature = data[0] | (data[1] << 8);
	cnt_remain = data[6];
	cnt_perc = data[7];

	temper = (float)temperature * DS1820_DECIMAL_STEPS;
	//*temper_val = temper;
	printf("temp=%.1f\n",temper);

	temp_read = temperature>>1;
	hres_temper = (float)temp_read-0.25+((cnt_perc-cnt_remain)/cnt_perc);
	printf("more_temp=%.1f\n",hres_temper);
	*temper_val = hres_temper;

	return _OK;
}


