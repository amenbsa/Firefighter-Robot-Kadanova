#ifndef MQ2_H
#define MQ2_H

#include "adc_driver.h"

#define MQ2_ADC_CHANNEL   ADC_CHANNEL_0  // placeholder — confirm against actual wiring

uint8_t MQ2_Init(void);
uint8_t MQ2_IsAvailable(void);
uint8_t MQ2_ReadRaw(uint16_t *out_raw);
uint8_t MQ2_ReadPPM(float *out_ppm);

#endif