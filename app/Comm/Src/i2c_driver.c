#include "i2c_driver.h"

void I2C_Driver_Init(void)
{
    MX_I2C1_Init();
}

HAL_StatusTypeDef I2C_Driver_Write(uint16_t dev_addr, uint8_t reg, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Write(&hi2c1, dev_addr, reg, I2C_MEMADD_SIZE_8BIT,
                              data, size, I2C_DEFAULT_TIMEOUT_MS);
}

HAL_StatusTypeDef I2C_Driver_Read(uint16_t dev_addr, uint8_t reg, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg, I2C_MEMADD_SIZE_8BIT,
                             data, size, I2C_DEFAULT_TIMEOUT_MS);
}

uint8_t I2C_Driver_IsDeviceReady(uint16_t dev_addr)
{
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, dev_addr, 2, I2C_DEFAULT_TIMEOUT_MS);
    return (status == HAL_OK) ? 1 : 0;
}