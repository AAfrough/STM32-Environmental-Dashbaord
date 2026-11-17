#ifndef BUTTONS_H
#define BUTTONS_H

#include "stm32f4xx_hal.h"

typedef enum
{
    BUTTON_NONE = 0,
    BUTTON_NEXT,
    BUTTON_PREV,
    BUTTON_SET,
	BUTTON_SET_LONG
} ButtonEvent;
void Buttons_Init(void);

ButtonEvent Buttons_GetEvent(void);

#endif
