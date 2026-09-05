#include "adc_driver.h"

void ADC_Driver_Init(void)
{
    MX_ADC1_Init();
}

HAL_StatusTypeDef ADC_Driver_Read(uint32_t channel, uint16_t *out_value)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_ADC_PollForConversion(&hadc1, 100);
    if (status != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return status; // propagates HAL_TIMEOUT distinctly from HAL_ERROR
    }

    *out_value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    return HAL_OK;
}