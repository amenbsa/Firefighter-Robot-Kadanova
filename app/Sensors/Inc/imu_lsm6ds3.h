#ifndef IMU_LSM6DS3_H
#define IMU_LSM6DS3_H

#include "i2c_driver.h"

#define LSM6DS3_I2C_ADDR   (0x6A << 1)  // HAL expects 8-bit address (7-bit addr shifted left)

typedef struct {
    float accel_x, accel_y, accel_z; // g
    float gyro_x, gyro_y, gyro_z;    // dps
} IMU_Data;

uint8_t IMU_Init(void);
uint8_t IMU_IsAvailable(void);
uint8_t IMU_Read(IMU_Data *out_data);

#endif