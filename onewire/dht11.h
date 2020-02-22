#ifndef DHT11_H
#define DHT11_H

#include "stm32f4xx_hal.h"


#define DHT11_SUCCESS         1
#define DHT11_ERROR_CHECKSUM  2
#define DHT11_ERROR_TIMEOUT   3

typedef struct DHT11_Dev {
	float temparature;
	float humidity;
	int err;
	GPIO_TypeDef* port;
	uint16_t pin;
} DHT11_Dev;

int DHT11_Init(struct DHT11_Dev* dev, GPIO_TypeDef* port, uint16_t pin);
int DHT11_Read(struct DHT11_Dev* dev);
char *DHT11_error_str(int err);

#endif /* DHT11_H */
