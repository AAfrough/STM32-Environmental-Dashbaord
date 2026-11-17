#include "dht11.h"

#define DHT11_PORT GPIOA
#define DHT11_PIN  GPIO_PIN_8

/* Initialize DWT cycle counter */
static void DHT11_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Microsecond delay using DWT */
static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = (HAL_RCC_GetHCLKFreq() / 1000000U) * us;

    while ((DWT->CYCCNT - start) < cycles);
}

/* Change PA8 to output Open Drain */
static void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/* Change PA8 to input */
static void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/* Wait for a specific pin level with timeout */
static HAL_StatusTypeDef DHT11_WaitForLevel(
    GPIO_PinState level,
    uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t timeout_cycles =
        (HAL_RCC_GetHCLKFreq() / 1000000U) * timeout_us;

    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != level)
    {
        if ((DWT->CYCCNT - start) > timeout_cycles)
            return HAL_TIMEOUT;
    }

    return HAL_OK;
}

/* Read one byte from DHT11 */
static HAL_StatusTypeDef DHT11_ReadByte(uint8_t *value)
{
    *value = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        /* Wait until the sensor starts the HIGH pulse */
        if (DHT11_WaitForLevel(GPIO_PIN_SET, 100) != HAL_OK)
            return HAL_TIMEOUT;

        uint32_t start = DWT->CYCCNT;

        /* Wait until the HIGH pulse ends */
        if (DHT11_WaitForLevel(GPIO_PIN_RESET, 100) != HAL_OK)
            return HAL_TIMEOUT;

        uint32_t duration = DWT->CYCCNT - start;

        /* Determine whether the bit is 0 or 1 */
        uint32_t threshold =
            HAL_RCC_GetHCLKFreq() / 1000000U * 50U;

        *value <<= 1;

        if (duration > threshold)
            *value |= 1;
    }

    return HAL_OK;
}

/* Main DHT11 reading function */
HAL_StatusTypeDef DHT11_Read(DHT11_Data *data)
{
    uint8_t buffer[5] = {0};

    if (data == NULL)
        return HAL_ERROR;

    DHT11_DWT_Init();

    /* Start signal */
    DHT11_SetPinOutput();

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    DHT11_DelayUs(30);

    /* Release the line and receive response */
    DHT11_SetPinInput();

    if (DHT11_WaitForLevel(GPIO_PIN_RESET, 100) != HAL_OK)
        return HAL_TIMEOUT;

    if (DHT11_WaitForLevel(GPIO_PIN_SET, 100) != HAL_OK)
        return HAL_TIMEOUT;

    if (DHT11_WaitForLevel(GPIO_PIN_RESET, 100) != HAL_OK)
        return HAL_TIMEOUT;

    /* Read 40 bits = 5 bytes */
    for (uint8_t i = 0; i < 5; i++)
    {
        if (DHT11_ReadByte(&buffer[i]) != HAL_OK)
            return HAL_TIMEOUT;
    }

    /* Checksum validation */
    uint8_t checksum =
        buffer[0] + buffer[1] + buffer[2] + buffer[3];

    if (checksum != buffer[4])
        return HAL_ERROR;

    /* Save measured values */
    data->humidity = buffer[0];
    data->temperature = buffer[2];

    return HAL_OK;
}
