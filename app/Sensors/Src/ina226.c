#include "ina226.h"

#define REG_CONFIG        0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE   0x02
#define REG_POWER         0x03
#define REG_CURRENT       0x04
#define REG_CALIBRATION   0x05
#define REG_MANUFACTURER_ID 0xFE

#define MANUFACTURER_ID_EXPECTED 0x5449 // "TI"

// Calibration depends on your actual shunt resistor value and expected max current.
// Placeholder assumes a 0.1 ohm shunt, ~3.2A max expected current.
// CAL = 0.00512 / (Current_LSB * Rshunt)
#define SHUNT_RESISTANCE_OHMS  0.1f
#define CURRENT_LSB_A          0.0001f  // 100uA/bit -> adjust based on real shunt/range
#define CALIBRATION_VALUE      ((uint16_t)(0.00512f / (CURRENT_LSB_A * SHUNT_RESISTANCE_OHMS)))

static uint8_t ina226_present = 0;

static HAL_StatusTypeDef INA226_ReadReg16(uint8_t reg, uint16_t *value)
{
    uint8_t raw[2];
    if (I2C_Driver_Read(INA226_I2C_ADDR, reg, raw, 2) != HAL_OK) {
        return HAL_ERROR;
    }
    *value = (uint16_t)(raw[0] << 8 | raw[1]); // INA226 is big-endian, unlike LSM6DS3
    return HAL_OK;
}

static HAL_StatusTypeDef INA226_WriteReg16(uint8_t reg, uint16_t value)
{
    uint8_t raw[2] = { (uint8_t)(value >> 8), (uint8_t)(value & 0xFF) };
    return I2C_Driver_Write(INA226_I2C_ADDR, reg, raw, 2);
}

uint8_t INA226_Init(void)
{
    uint16_t manufacturer_id = 0;

    if (INA226_ReadReg16(REG_MANUFACTURER_ID, &manufacturer_id) != HAL_OK) {
        ina226_present = 0;
        return 0;
    }

    if (manufacturer_id != MANUFACTURER_ID_EXPECTED) {
        ina226_present = 0;
        return 0;
    }

    // Config: default reset value with continuous shunt+bus voltage conversion (0x4127 is a common default)
    if (INA226_WriteReg16(REG_CONFIG, 0x4127) != HAL_OK) {
        ina226_present = 0;
        return 0;
    }

    if (INA226_WriteReg16(REG_CALIBRATION, CALIBRATION_VALUE) != HAL_OK) {
        ina226_present = 0;
        return 0;
    }

    ina226_present = 1;
    return 1;
}

uint8_t INA226_IsAvailable(void)
{
    return I2C_Driver_IsDeviceReady(INA226_I2C_ADDR) && ina226_present;
}

uint8_t INA226_Read(INA226_Data *out_data)
{
    if (!ina226_present) {
        return 0;
    }

    uint16_t bus_raw, current_raw, power_raw;

    if (INA226_ReadReg16(REG_BUS_VOLTAGE, &bus_raw) != HAL_OK) return 0;
    if (INA226_ReadReg16(REG_CURRENT, &current_raw) != HAL_OK) return 0;
    if (INA226_ReadReg16(REG_POWER, &power_raw) != HAL_OK) return 0;

    out_data->bus_voltage_v = bus_raw * 0.00125f;              // 1.25mV/LSB, fixed by datasheet
    out_data->current_a     = (int16_t)current_raw * CURRENT_LSB_A;
    out_data->power_w       = power_raw * CURRENT_LSB_A * 25.0f; // Power_LSB = 25 * Current_LSB

    return 1;
}