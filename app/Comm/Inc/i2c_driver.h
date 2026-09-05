// i2c_driver.h
#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include "i2c.h"   // CubeMX-generated, once I2C1 is enabled in .ioc

#define I2C_DEFAULT_TIMEOUT_MS  100

void I2C_Driver_Init(void);
HAL_StatusTypeDef I2C_Driver_Write(uint16_t dev_addr, uint8_t reg, uint8_t *data, uint16_t size);
HAL_StatusTypeDef I2C_Driver_Read(uint16_t dev_addr, uint8_t reg, uint8_t *data, uint16_t size);
uint8_t I2C_Driver_IsDeviceReady(uint16_t dev_addr);

#endif