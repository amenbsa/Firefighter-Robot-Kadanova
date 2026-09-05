#include "mq2.h"

static uint8_t mq2_present = 0;

uint8_t MQ2_Init(void)
{
    // No dedicated init sequence for MQ-2 itself — it's a passive analog
    // sensor. ADC1 is already initialized by ADC_Driver_Init(). We just
    // do one test read to confirm the ADC channel is responsive.
    uint16_t test_value;
    if (ADC_Driver_Read(MQ2_ADC_CHANNEL, &test_value) != HAL_OK) {
        mq2_present = 0;
        return 0;
    }

    mq2_present = 1;
    return 1;
}

uint8_t MQ2_IsAvailable(void)
{
    return mq2_present;
}

uint8_t MQ2_ReadRaw(uint16_t *out_raw)
{
    if (!mq2_present) {
        return 0;
    }
    return (ADC_Driver_Read(MQ2_ADC_CHANNEL, out_raw) == HAL_OK);
}

uint8_t MQ2_ReadPPM(float *out_ppm)
{
    uint16_t raw;
    if (!MQ2_ReadRaw(&raw)) {
        return 0;
    }

    // Placeholder linear mapping — MQ-2 actually requires a logarithmic
    // curve fit against Rs/Ro ratio per its datasheet graphs, and depends
    // on your specific load resistor and calibration in clean air.
    // This is NOT accurate yet, just a placeholder so the pipeline compiles.
    *out_ppm = (raw / 4095.0f) * 10000.0f; // 12-bit ADC, arbitrary scale

    return 1;
}