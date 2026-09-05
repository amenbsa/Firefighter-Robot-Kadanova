#include "mlx90614.h"

#define REG_TA      0x06  // Ambient temperature
#define REG_TOBJ1   0x07  // Object temperature (channel 1)

static uint8_t mlx90614_present = 0;

// MLX90614 uses SMBus, which has a PEC (packet error check) byte per transaction.
// HAL's Mem_Read doesn't validate PEC on its own — we read 3 bytes (2 data + 1 PEC)
// and only check the data here, not the PEC, for simplicity. See note below.
static HAL_StatusTypeDef MLX90614_ReadTemp(uint8_t reg, float *out_celsius)
{
    uint8_t raw[3]; // low byte, high byte, PEC

    if (I2C_Driver_Read(MLX90614_I2C_ADDR, reg, raw, 3) != HAL_OK) {
        return HAL_ERROR;
    }

    uint16_t raw_temp = (uint16_t)(raw[1] << 8 | raw[0]); // little-endian

    // Check the error flag (bit 15) per datasheet — indicates a sensor fault
    if (raw_temp & 0x8000) {
        return HAL_ERROR;
    }

    // Datasheet formula: Temp(K) = raw * 0.02, then convert to Celsius
    float temp_kelvin = raw_temp * 0.02f;
    *out_celsius = temp_kelvin - 273.15f;

    return HAL_OK;
}

uint8_t MLX90614_Init(void)
{
    // MLX90614 has no WHO_AM_I-style register — presence is confirmed by
    // a successful read returning a plausible value, done here as a sanity check.
    float test_temp;
    if (MLX90614_ReadTemp(REG_TA, &test_temp) != HAL_OK) {
        mlx90614_present = 0;
        return 0;
    }

    // Sanity range check: ambient temp should realistically be within a plausible range
    if (test_temp < -50.0f || test_temp > 150.0f) {
        mlx90614_present = 0;
        return 0;
    }

    mlx90614_present = 1;
    return 1;
}

uint8_t MLX90614_IsAvailable(void)
{
    return I2C_Driver_IsDeviceReady(MLX90614_I2C_ADDR) && mlx90614_present;
}

uint8_t MLX90614_Read(MLX90614_Data *out_data)
{
    if (!mlx90614_present) {
        return 0;
    }

    if (MLX90614_ReadTemp(REG_TA, &out_data->ambient_temp_c) != HAL_OK) return 0;
    if (MLX90614_ReadTemp(REG_TOBJ1, &out_data->object_temp_c) != HAL_OK) return 0;

    return 1;
}
// TODO (before real hardware testing):
// 1. No WHO_AM_I-style identity register exists on this sensor — presence is
//    currently inferred via a plausible-range sanity check on ambient temp,
//    which is weaker than the identity checks used in other sensor drivers.
// 2. PEC (SMBus CRC-8) byte is read but not validated — corrupted readings
//    could currently pass through undetected. Implement CRC-8 check over
//    [addr, reg, addr|0x01, data_low, data_high] before trusting readings
//    in a real fire-detection decision path.