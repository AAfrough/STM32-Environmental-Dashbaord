#include "buttons.h"

static volatile uint8_t nextFlag = 0;
static volatile uint8_t prevFlag = 0;

static uint32_t lastNextTick = 0;
static uint32_t lastPrevTick = 0;

static volatile uint8_t setFlag = 0;
static volatile uint8_t setLongFlag = 0;
static volatile uint8_t setPressed = 0;

static uint32_t lastSetTick = 0;

static uint32_t setPressTick = 0;

#define DEBOUNCE_TIME 300

void Buttons_Init(void)
{
    nextFlag = 0;
    prevFlag = 0;
    setFlag = 0;
    setLongFlag = 0;
    setPressed = 0;
    setPressTick = 0;
    lastSetTick = HAL_GetTick() - DEBOUNCE_TIME;
    // Allow the first button press immediately
    lastNextTick = HAL_GetTick() - DEBOUNCE_TIME;
    lastPrevTick = HAL_GetTick() - DEBOUNCE_TIME;
}


// HAL external interrupt callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t now = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_0)
    {
        if ((uint32_t)(now - lastNextTick) >= DEBOUNCE_TIME)
        {
            nextFlag = 1;
            lastNextTick = now;
        }
    }
    else if (GPIO_Pin == GPIO_PIN_1)
    {
        if ((uint32_t)(now - lastPrevTick) >= DEBOUNCE_TIME)
        {
            prevFlag = 1;
            lastPrevTick = now;
        }
    }
    else if (GPIO_Pin == GPIO_PIN_2)
    {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET)
        {
            if (!setPressed)
            {
                setPressed = 1;
                setPressTick = now;
            }
        }
        else
        {
            if (setPressed)
            {
                uint32_t duration = now - setPressTick;

                if (duration >= 1000)
                    setLongFlag = 1;
                else
                    setFlag = 1;

                setPressed = 0;
            }
        }
    }
}


// Read and clear button event flags
ButtonEvent Buttons_GetEvent(void)
{
    if (setLongFlag)
    {
        setLongFlag = 0;
        return BUTTON_SET_LONG;
    }

    if (setFlag)
    {
        setFlag = 0;
        return BUTTON_SET;
    }

    if (nextFlag)
    {
        nextFlag = 0;
        return BUTTON_NEXT;
    }

    if (prevFlag)
    {
        prevFlag = 0;
        return BUTTON_PREV;
    }

    return BUTTON_NONE;
}
