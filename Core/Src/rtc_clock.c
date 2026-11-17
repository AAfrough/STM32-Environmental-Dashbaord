#include "main.h"
#include "rtc_clock.h"

HAL_StatusTypeDef RTCClock_GetTime(RTC_TimeTypeDef *time)
{
    return HAL_RTC_GetTime(&hrtc, time, RTC_FORMAT_BIN);
}

HAL_StatusTypeDef RTCClock_GetDate(RTC_DateTypeDef *date)
{
    return HAL_RTC_GetDate(&hrtc, date, RTC_FORMAT_BIN);
}

HAL_StatusTypeDef RTCClock_SetTime(
    uint8_t hours,
    uint8_t minutes,
    uint8_t seconds)
{
    RTC_TimeTypeDef time = {0};

    time.Hours = hours;
    time.Minutes = minutes;
    time.Seconds = seconds;
    time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation = RTC_STOREOPERATION_RESET;

    return HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
}

HAL_StatusTypeDef RTCClock_SetDate(
    uint8_t day,
    uint8_t month,
    uint8_t year)
{
    RTC_DateTypeDef date = {0};

    date.Date = day;
    date.Month = month;
    date.Year = year;

    date.WeekDay = RTC_WEEKDAY_MONDAY;

    return HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
}
