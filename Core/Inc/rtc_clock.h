#ifndef RTC_CLOCK_H
#define RTC_CLOCK_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef RTCClock_GetTime(
		RTC_TimeTypeDef *time);

HAL_StatusTypeDef RTCClock_GetDate(
		RTC_DateTypeDef *date);

HAL_StatusTypeDef RTCClock_SetTime(
    uint8_t hours,
    uint8_t minutes,
    uint8_t seconds
);

HAL_StatusTypeDef RTCClock_SetDate(
    uint8_t day,
    uint8_t month,
    uint8_t year
);


#endif
