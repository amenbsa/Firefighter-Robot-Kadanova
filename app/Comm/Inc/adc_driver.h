#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "adc.h"

void ADC_Driver_Init(void);
HAL_StatusTypeDef ADC_Driver_Read(uint32_t channel, uint16_t *out_value);

#endif