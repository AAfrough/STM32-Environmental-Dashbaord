#ifndef SOUND_H
#define SOUND_H

#include "stm32f4xx_hal.h"

void Sound_Init(void);
void Sound_Process(void);
uint8_t Sound_GetLevel(void);

#endif
