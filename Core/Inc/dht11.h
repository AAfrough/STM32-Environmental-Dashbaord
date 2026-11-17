#ifndef DHT11_H
#define DHT11_H

#include "stm32f4xx_hal.h"

typedef struct
{
    uint8_t temperature;
    uint8_t humidity;
} DHT11_Data;

HAL_StatusTypeDef DHT11_Read(DHT11_Data *data);

#endif
