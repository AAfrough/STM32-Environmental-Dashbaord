#ifndef LDR_H
#define LDR_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef LDR_Read(uint16_t *value);

#endif
