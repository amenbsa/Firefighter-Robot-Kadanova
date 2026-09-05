// TODO: this is a MINIMAL, UNCALIBRATED driver — no SPAD management, no
// temperature/voltage reference calibration. ST's official API (STSW-IMG005)
// performs these steps and should be integrated before trusting this for
// real proximity/obstacle decisions. Readings here may be less accurate
// and could drift across temperature/lighting conditions. See earlier
// discussion for why full calibration was deferred.

#include "vl53l0x.h"

#define REG_MODEL_ID           0xC0  // expected 0xEE on genuine VL53L0X
#define REG_SYSRANGE_START     0x00
#define REG_RESULT_RANGE_STATUS 0x14 // range result block starts here
#define REG_RESULT_RANGE_MM    0x1E  // 2 bytes, big-endian, within the result block

#define MODEL_ID_EXPECTED 0xEE

static uint8_t vl53l0x_present = 0;

uint8_t VL53L0X_Init(void)
{
    uint8_t model_id = 0;

    if (I2C_Driver_Read(VL53L0X_I2C_ADDR, REG_MODEL_ID, &model_id, 1) != HAL_OK) {
        vl53l0x_present = 0;
        return 0;
    }

    if (model_id != MODEL_ID_EXPECTED) {
        vl53l0x_present = 0;
        return 0;
    }

    // No SPAD/ref calibration performed here — sensor is used with its
    // power-on defaults, which may not be optimal. See TODO above.
    vl53l0x_present = 1;
    return 1;
}

uint8_t VL53L0X_IsAvailable(void)
{
    return I2C_Driver_IsDeviceReady(VL53L0X_I2C_ADDR) && vl53l0x_present;
}

uint8_t VL53L0X_ReadDistanceMM(uint16_t *out_mm)
{
    if (!vl53l0x_present) {
        return 0;
    }

    // Start a single-shot ranging measurement
    uint8_t start_cmd = 0x01;
    if (I2C_Driver_Write(VL53L0X_I2C_ADDR, REG_SYSRANGE_START, &start_cmd, 1) != HAL_OK) {
        return 0;
    }

    // Poll for completion — no interrupt/GPIO ready-pin handling here,
    // just a bounded retry loop. A production driver would use the
    // sensor's GPIO1 interrupt pin instead of polling.
    uint8_t status = 0;
    uint32_t start_tick = HAL_GetTick();
    const uint32_t timeout_ms = 100;

    do {
        if (I2C_Driver_Read(VL53L0X_I2C_ADDR, REG_RESULT_RANGE_STATUS, &status, 1) != HAL_OK) {
            return 0;
        }
        if (HAL_GetTick() - start_tick > timeout_ms) {
            return 0; // measurement never completed — treat as failure, not 0mm
        }
    } while ((status & 0x01) == 0); // bit 0 = data ready, per known register map

    uint8_t range_raw[2];
    if (I2C_Driver_Read(VL53L0X_I2C_ADDR, REG_RESULT_RANGE_MM, range_raw, 2) != HAL_OK) {
        return 0;
    }

    *out_mm = (uint16_t)(range_raw[0] << 8 | range_raw[1]); // big-endian

    return 1;
}