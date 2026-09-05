#ifndef VL53L0X_H
#define VL53L0X_H

#include "i2c_driver.h"

#define VL53L0X_I2C_ADDR   (0x29 << 1)  // fixed factory default, not pin-configurable

uint8_t VL53L0X_Init(void);
uint8_t VL53L0X_IsAvailable(void);
uint8_t VL53L0X_ReadDistanceMM(uint16_t *out_mm);

#endif