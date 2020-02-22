
#include "stdio.h"
#include "dht11.h"
#include "onewr_port.h"


int DHT11_Init(struct DHT11_Dev* dev, GPIO_TypeDef* port, uint16_t pin) {

	dev->port = port;
	dev->pin = pin;

	return 0;
}

char *DHT11_error_str(int err){
	switch(err){
	case DHT11_ERROR_TIMEOUT: return "timeout";
	case DHT11_ERROR_CHECKSUM: return "checksum";
	case DHT11_SUCCESS: return "success";
	}
	return "unknown";
}


int DHT11_Read(struct DHT11_Dev* dev) {
	uint32_t micro_cnt;
	
	//Initialization
	uint8_t i, j, temp;
	uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//Generate START condition
	GPIO_SetPinAsOutput(dev->port,dev->pin);
	//Put LOW for at least 18ms
	HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_RESET);
	//wait 18ms
	Delay_micro(18000);
	//Put HIGH for 20-40us
	HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_SET);
	//wait 40us
	Delay_micro(40);
	//End start condition

	//Input mode to receive data
	GPIO_SetPinAsInput(dev->port,dev->pin);
	//DHT11 ACK
	//should be LOW for at least 80us
	micro_cnt = Get_micros();
	while(!HAL_GPIO_ReadPin(dev->port, dev->pin)) {
		if(Get_micros() - micro_cnt > 100){
			dev->err = DHT11_ERROR_TIMEOUT;
			return DHT11_ERROR_TIMEOUT;
		}
	}
	
	//should be HIGH for at least 80us
	micro_cnt = Get_micros();
	while(HAL_GPIO_ReadPin(dev->port, dev->pin)) {
		if(Get_micros() - micro_cnt > 100){
			dev->err = DHT11_ERROR_TIMEOUT;
			return DHT11_ERROR_TIMEOUT;
		}
	}
	
	//Read 40 bits (8*5)
	for(j = 0; j < 5; ++j) {
		for(i = 0; i < 8; ++i) {
			
			//LOW for 50us
			while(!HAL_GPIO_ReadPin(dev->port, dev->pin));
			//Start counter
			micro_cnt = Get_micros();
			//HIGH for 26-28us = 0 / 70us = 1
			while(HAL_GPIO_ReadPin(dev->port, dev->pin)) {
				if(Get_micros() - micro_cnt > 100){
					dev->err = DHT11_ERROR_TIMEOUT;
					return DHT11_ERROR_TIMEOUT;
				}
			}
			//Calc amount of time passed
			temp = Get_micros() - micro_cnt;
			
			//shift 0
			data[j] = data[j] << 1;
			//if > 30us it's 1
			if(temp > 40)
				data[j] = data[j]+1;
		}
	}

	printf("DHT11 Data: %X,%X,%X,%X,%X\n",data[0],data[1],data[2],data[3],data[4]);
	//verify the Checksum
	if(data[4] != (data[0] + data[1] + data[2] + data[3])){
		dev->err = DHT11_ERROR_CHECKSUM;
		return DHT11_ERROR_CHECKSUM;
	}
	
	//set data
	dev->humidity = (float)data[0] + (float)data[1]/10;
	dev->temparature = (float)data[2] + (float)data[3]/10;
	dev->err = DHT11_SUCCESS;
	
	return DHT11_SUCCESS;
}
