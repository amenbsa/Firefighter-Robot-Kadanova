// TODO: register map, resolution, and MANUFACTURER_ID/DEVICE_ID values below
// were drafted from general ADI thermal-sensor conventions, not verified
// against the actual ADT7482 datasheet. Confirm before trusting readings.
#include "adt7482.h"

#define REG_LOCAL_TEMP_HIGH    0x00
#define REG_REMOTE_TEMP_HIGH   0x01
#define REG_STATUS             0x02
#define REG_CONFIG1            0x03
#define REG_LOCAL_TEMP_LOW     0x15
#define REG_REMOTE_TEMP_LOW    0x10
#define REG_MANUFACTURER_ID    0xFE
#define REG_DEVICE_ID          0xFF

#define MANUFACTURER_ID_EXPECTED 0x41 // ADI
#define DEVICE_ID_EXPECTED       0x71 // ADT7482 specific — verify against your exact datasheet revision

static uint8_t adt7482_present = 0;

uint8_t ADT7482_Init(void)
{
    uint8_t manufacturer_id = 0;
    uint8_t device_id = 0;

    if (I2C_Driver_Read(ADT7482_I2C_ADDR, REG_MANUFACTURER_ID, &manufacturer_id, 1) != HAL_OK) {
        adt7482_present = 0;
        return 0;
    }
    if (I2C_Driver_Read(ADT7482_I2C_ADDR, REG_DEVICE_ID, &device_id, 1) != HAL_OK) {
        adt7482_present = 0;
        return 0;
    }

    if (manufacturer_id != MANUFACTURER_ID_EXPECTED || device_id != DEVICE_ID_EXPECTED) {
        adt7482_present = 0;
        return 0;
    }

    // CONFIG1: default operation, continuous conversion (0x00 = normal/default mode)
    uint8_t config1 = 0x00;
    if (I2C_Driver_Write(ADT7482_I2C_ADDR, REG_CONFIG1, &config1, 1) != HAL_OK) {
        adt7482_present = 0;
        return 0;
    }

    adt7482_present = 1;
    return 1;
}

uint8_t ADT7482_IsAvailable(void)
{
    return I2C_Driver_IsDeviceReady(ADT7482_I2C_ADDR) && adt7482_present;
}

uint8_t ADT7482_Read(ADT7482_Data *out_data)
{
    if (!adt7482_present) {
        return 0;
    }

    uint8_t local_high, local_low, remote_high, remote_low;

    if (I2C_Driver_Read(ADT7482_I2C_ADDR, REG_LOCAL_TEMP_HIGH, &local_high, 1) != HAL_OK) return 0;
    if (I2C_Driver_Read(ADT7482_I2C_ADDR, REG_LOCAL_TEMP_LOW, &local_low, 1) != HAL_OK) return 0;
    if (I2C_Driver_Read(ADT7482_I2C_ADDR, REG_REMOTE_TEMP_HIGH, &remote_high, 1) != HAL_OK) return 0;
    if (I2C_Driver_Read(ADT7482_I2C_ADDR, REG_REMOTE_TEMP_LOW, &remote_low, 1) != HAL_OK) return 0;

    // High byte = integer part (signed), low byte upper bits = fractional (0.0625/LSB), per datasheet
    int8_t local_int  = (int8_t)local_high;
    int8_t remote_int = (int8_t)remote_high;

    float local_frac  = (local_low >> 4) * 0.0625f;
    float remote_frac = (remote_low >> 4) * 0.0625f;

    out_data->local_temp_c  = local_int + local_frac;
    out_data->remote_temp_c = remote_int + remote_frac;

    return 1;
}