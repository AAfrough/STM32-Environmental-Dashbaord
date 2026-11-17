#include "main.h"
#include "ldr.h"

HAL_StatusTypeDef LDR_Read(uint16_t *value)
{
    if (value == NULL)
        return HAL_ERROR;

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
        return HAL_ERROR;

    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return HAL_TIMEOUT;
    }

    *value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return HAL_OK;
}
