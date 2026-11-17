#include "main.h"
#include "dashboard.h"
#include "buttons.h"
#include "rtc_clock.h"
#include "dht11.h"
#include "lcd.h"
#include "ldr.h"
#include "sound.h"
#include <stdio.h>
#include <stdint.h>

typedef enum
{
    PAGE_CLOCK = 0,
    PAGE_TEMPERATURE,
    PAGE_HUMIDITY,
    PAGE_LIGHT,
	PAGE_SOUND
} DashboardPage;

typedef enum
{
    SET_HOUR = 0,
    SET_MINUTE,
    SET_SECOND,
    SET_DAY,
    SET_MONTH,
    SET_YEAR
} SettingField;

typedef enum
{
    MODE_NORMAL = 0,
    MODE_SETTINGS
} DashboardMode;

/* Global module variables */

static DashboardMode currentMode = MODE_NORMAL;

static SettingField currentField = SET_HOUR;

/* Temporary time and date */
static RTC_TimeTypeDef editTime;
static RTC_DateTypeDef editDate;

static DashboardPage currentPage = PAGE_CLOCK;

static RTC_TimeTypeDef rtcTime;
static RTC_DateTypeDef rtcDate;

static DHT11_Data sensorData;

static char buffer[17];

static uint32_t lastClockUpdate = 0;
static uint32_t lastSensorUpdate = 0;

static uint16_t ldrValue = 0;
static uint32_t lastLdrUpdate = 0;

static uint32_t lastSoundUpdate = 0;

/* Function declarations */
static void Dashboard_ShowPage(void);
static void Dashboard_UpdateClock(void);
static void Dashboard_UpdateSensor(void);
static void Dashboard_UpdateLight(void);
static void Dashboard_UpdateSound(void);

static void Settings_Enter(void);
static void Settings_Show(void);
static void Settings_Process(ButtonEvent event);
static void Settings_Save(void);
static void Settings_Exit(void);
static uint8_t Settings_GetMaxDay(void);

/* Display selected page */
static void Dashboard_ShowPage(void)
{
    LCD_Clear();

    switch (currentPage)
    {
        case PAGE_CLOCK:
            LCD_SetCursor(0, 0);
            LCD_Print("CLOCK:");
            LCD_SetCursor(1, 0);
			LCD_Print("DATE:");
            break;

        case PAGE_TEMPERATURE:
            LCD_SetCursor(0, 0);
            LCD_Print("Temp:");
            LCD_SetCursor(1, 1);
            LCD_Print("Reading...");
            break;

        case PAGE_HUMIDITY:
            LCD_SetCursor(0, 0);
            LCD_Print("Humidity:");
            LCD_SetCursor(1, 1);
            LCD_Print("Reading...");
            break;

        case PAGE_LIGHT:
            LCD_SetCursor(0, 0);
            LCD_Print("Light:");
            LCD_SetCursor(1, 1);
            LCD_Print("Reading...");
            break;

        case PAGE_SOUND:
            LCD_SetCursor(0, 0);
            LCD_Print("SOUND:");
            LCD_SetCursor(1, 1);
            LCD_Print("Reading...");
            break;
    }
}

/*Clock Settings */
static void Settings_Enter(void)
{
    RTCClock_GetTime(&editTime);
    RTCClock_GetDate(&editDate);

    currentField = SET_HOUR;
    currentMode = MODE_SETTINGS;

    Settings_Show();
}

static uint8_t Settings_GetMaxDay(void)
{
    switch (editDate.Month)
    {
        case RTC_MONTH_APRIL:
        case RTC_MONTH_JUNE:
        case RTC_MONTH_SEPTEMBER:
        case RTC_MONTH_NOVEMBER:
            return 30;

        case RTC_MONTH_FEBRUARY:
            if ((editDate.Year % 4) == 0)
                return 29;
            else
                return 28;

        default:
            return 31;
    }
}

static void Settings_Show(void)
{
    LCD_Clear();

    switch (currentField)
    {
        case SET_HOUR:
            sprintf(buffer, "Hour: %02u", editTime.Hours);
            break;

        case SET_MINUTE:
            sprintf(buffer, "Minute: %02u", editTime.Minutes);
            break;

        case SET_SECOND:
            sprintf(buffer, "Second: %02u", editTime.Seconds);
            break;

        case SET_DAY:
            sprintf(buffer, "Day: %02u", editDate.Date);
            break;

        case SET_MONTH:
            sprintf(buffer, "Month: %02u", editDate.Month);
            break;

        case SET_YEAR:
            sprintf(buffer, "Year: 20%02u", editDate.Year);
            break;
    }

    LCD_SetCursor(0, 2);
    LCD_Print("SET DATE/TIME");

    LCD_SetCursor(1, 0);
    LCD_Print(buffer);
}

static void Settings_Adjust(int8_t direction)
{
    switch (currentField)
    {
        case SET_HOUR:
            editTime.Hours = (editTime.Hours + direction + 24) % 24;
            break;

        case SET_MINUTE:
            editTime.Minutes = (editTime.Minutes + direction + 60) % 60;
            break;

        case SET_SECOND:
            editTime.Seconds = (editTime.Seconds + direction + 60) % 60;
            break;

        case SET_DAY:
        {
            uint8_t maxDay = Settings_GetMaxDay();

            if (direction > 0)
                editDate.Date = (editDate.Date >= maxDay) ? 1 : editDate.Date + 1;
            else
                editDate.Date = (editDate.Date <= 1) ? maxDay : editDate.Date - 1;

            break;
        }

        case SET_MONTH:
            if (direction > 0)
                editDate.Month = (editDate.Month >= 12) ? 1 : editDate.Month + 1;
            else
                editDate.Month = (editDate.Month <= 1) ? 12 : editDate.Month - 1;

            break;

        case SET_YEAR:
            if (direction > 0)
                editDate.Year = (editDate.Year >= 99) ? 0 : editDate.Year + 1;
            else
                editDate.Year = (editDate.Year == 0) ? 99 : editDate.Year - 1;

            break;
    }

    /* Correct day if the selected month/year has fewer days */
    if (editDate.Date > Settings_GetMaxDay())
        editDate.Date = Settings_GetMaxDay();

    Settings_Show();
}

static void Settings_Save(void)
{
    HAL_StatusTypeDef timeStatus;
    HAL_StatusTypeDef dateStatus;

    dateStatus = RTCClock_SetDate(
        editDate.Date,
        editDate.Month,
        editDate.Year
    );

    timeStatus = RTCClock_SetTime(
        editTime.Hours,
        editTime.Minutes,
        editTime.Seconds
    );

    if (dateStatus == HAL_OK && timeStatus == HAL_OK)
    {
        Settings_Exit();
    }
    else
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("RTC SAVE ERROR");
        LCD_SetCursor(0, 1);
        LCD_Print("Try Again");
    }
}

static void Settings_Exit(void)
{
    currentMode = MODE_NORMAL;

    Dashboard_ShowPage();

    lastClockUpdate = 0;
    lastSensorUpdate = 0;
    lastLdrUpdate = 0;
}

static void Settings_Process(ButtonEvent event)
{
    if (event == BUTTON_SET_LONG)
    {
        Settings_Exit();
        return;
    }

    if (event == BUTTON_NEXT)
    {
        Settings_Adjust(1);
    }
    else if (event == BUTTON_PREV)
    {
        Settings_Adjust(-1);
    }
    else if (event == BUTTON_SET)
    {
        if (currentField < SET_YEAR)
        {
            currentField++;
            Settings_Show();
        }
        else
        {
            Settings_Save();
        }
    }
}

/* Initialize dashboard */
void Dashboard_Init(void)
{
    LCD_Init();
    Buttons_Init();
    Sound_Init();

//    currentPage = PAGE_CLOCK;
//    LCD_SetCursor(0, 0);

    Dashboard_ShowPage();


    lastClockUpdate = 0;
    lastSensorUpdate = 0;
    lastLdrUpdate = 0;
    currentMode = MODE_NORMAL;
}


/* Main dashboard task */
void Dashboard_Process(void)
{
    ButtonEvent event = Buttons_GetEvent();
    Sound_Process();

    if (currentMode == MODE_SETTINGS)
    {
        Settings_Process(event);
        return;
    }

    if (event == BUTTON_SET)
    {
        Settings_Enter();
        return;
    }

    if (event == BUTTON_NEXT)
    {
        currentPage++;

        if (currentPage > PAGE_SOUND)
            currentPage = PAGE_CLOCK;

        Dashboard_ShowPage();

        lastClockUpdate = 0;
        lastSensorUpdate = 0;
        lastLdrUpdate = 0;
    }
    else if (event == BUTTON_PREV)
    {
        if (currentPage == PAGE_CLOCK)
            currentPage = PAGE_SOUND;
        else
            currentPage--;

        Dashboard_ShowPage();

        lastClockUpdate = 0;
        lastSensorUpdate = 0;
        lastLdrUpdate = 0;
    }

    /* Clock page */
    if (currentPage == PAGE_CLOCK)
    {
        if (HAL_GetTick() - lastClockUpdate >= 1000)
        {
            Dashboard_UpdateClock();
            lastClockUpdate = HAL_GetTick();
        }
    }

    /* Sensor pages */
    if (currentPage == PAGE_TEMPERATURE ||
        currentPage == PAGE_HUMIDITY)
    {
        if (HAL_GetTick() - lastSensorUpdate >= 2000)
        {
            Dashboard_UpdateSensor();
            lastSensorUpdate = HAL_GetTick();
        }
    }
    if (currentPage == PAGE_LIGHT)
    {
        if (HAL_GetTick() - lastLdrUpdate >= 500)
        {
            Dashboard_UpdateLight();
            lastLdrUpdate = HAL_GetTick();
        }
    }


    if (currentPage == PAGE_SOUND)
    {
        if (HAL_GetTick() - lastSoundUpdate >= 200)
        {
            lastSoundUpdate = HAL_GetTick();
            Dashboard_UpdateSound();
        }
    }
}

/* Update clock */
static void Dashboard_UpdateClock(void)
{
    RTCClock_GetTime(&rtcTime);
    RTCClock_GetDate(&rtcDate);

    sprintf(buffer, "%02d:%02d:%02d",
            rtcTime.Hours,
            rtcTime.Minutes,
            rtcTime.Seconds);

    LCD_SetCursor(0, 8);
    LCD_Print(buffer);

    sprintf(buffer, "20%02u/%02u/%02u",
    		rtcDate.Year,
            rtcDate.Month,
			rtcDate.Date);


    LCD_SetCursor(1, 6);
    LCD_Print(buffer);
}


/* Update light */
static void Dashboard_UpdateLight(void)
{
    if (LDR_Read(&ldrValue) == HAL_OK)
    {
        uint16_t percentage =
            (ldrValue * 100U) / 3700U;

        sprintf(buffer, "%5u%%",
        		percentage);
    }
    else
    {
        sprintf(buffer, "ADC Error");
    }

    LCD_SetCursor(0, 5);
    LCD_Print(buffer);
}

/* Update temperature and humidity */
static void Dashboard_UpdateSensor(void)
{
    if (DHT11_Read(&sensorData) == HAL_OK)
    {
        if (currentPage == PAGE_TEMPERATURE)
        {
            sprintf(buffer, "Temp: %u C",
                    (unsigned int)sensorData.temperature);
        }
        else if (currentPage == PAGE_HUMIDITY)
        {
            sprintf(buffer, "Humidity: %u%%",
                    (unsigned int)sensorData.humidity);
        }
    }
    else
    {
        sprintf(buffer, "Sensor Error");
    }

    LCD_SetCursor(0, 0);
    LCD_Print(buffer);
}

/* Update sound */
static void Dashboard_UpdateSound(void)
{
    uint8_t level = Sound_GetLevel();

    sprintf(buffer, "%3u%%", level);

    LCD_SetCursor(0, 6);
    LCD_Print(buffer);
}
