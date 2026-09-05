#include "imu_lsm6ds3.h"

// LSM6DS3 register map (partial — only what we need)
#define REG_WHO_AM_I     0x0F
#define REG_CTRL1_XL     0x10  // accel ODR/scale
#define REG_CTRL2_G      0x11  // gyro ODR/scale
#define REG_OUTX_L_G     0x22  // gyro data start (6 bytes: X,Y,Z as int16 LE)
#define REG_OUTX_L_XL    0x28  // accel data start (6 bytes: X,Y,Z as int16 LE)

#define WHO_AM_I_EXPECTED 0x69

// Sensitivity for the config we're using below (±2g accel, ±245dps gyro)
#define ACCEL_SENSITIVITY  0.061f   // mg/LSB -> divide by 1000 for g
#define GYRO_SENSITIVITY   8.75f    // mdps/LSB -> divide by 1000 for dps

static uint8_t imu_present = 0;

uint8_t IMU_Init(void)
{
    uint8_t who_am_i = 0;

    if (I2C_Driver_Read(LSM6DS3_I2C_ADDR, REG_WHO_AM_I, &who_am_i, 1) != HAL_OK) {
        imu_present = 0;
        return 0;
    }

    if (who_am_i != WHO_AM_I_EXPECTED) {
        imu_present = 0; // wrong device on this address, or nothing there
        return 0;
    }

    // CTRL1_XL: ODR 104 Hz, ±2g full scale  -> 0x40
    uint8_t ctrl1_xl = 0x40;
    // CTRL2_G:  ODR 104 Hz, ±245 dps full scale -> 0x40
    uint8_t ctrl2_g  = 0x40;

    if (I2C_Driver_Write(LSM6DS3_I2C_ADDR, REG_CTRL1_XL, &ctrl1_xl, 1) != HAL_OK) {
        imu_present = 0;
        return 0;
    }
    if (I2C_Driver_Write(LSM6DS3_I2C_ADDR, REG_CTRL2_G, &ctrl2_g, 1) != HAL_OK) {
        imu_present = 0;
        return 0;
    }

    imu_present = 1;
    return 1;
}

uint8_t IMU_IsAvailable(void)
{
    return I2C_Driver_IsDeviceReady(LSM6DS3_I2C_ADDR) && imu_present;
}

uint8_t IMU_Read(IMU_Data *out_data)
{
    if (!imu_present) {
        return 0; // don't even attempt — matches graceful-degradation pattern
    }

    uint8_t gyro_raw[6];
    uint8_t accel_raw[6];

    if (I2C_Driver_Read(LSM6DS3_I2C_ADDR, REG_OUTX_L_G, gyro_raw, 6) != HAL_OK) {
        return 0;
    }
    if (I2C_Driver_Read(LSM6DS3_I2C_ADDR, REG_OUTX_L_XL, accel_raw, 6) != HAL_OK) {
        return 0;
    }

    int16_t gx = (int16_t)(gyro_raw[1] << 8 | gyro_raw[0]);
    int16_t gy = (int16_t)(gyro_raw[3] << 8 | gyro_raw[2]);
    int16_t gz = (int16_t)(gyro_raw[5] << 8 | gyro_raw[4]);

    int16_t ax = (int16_t)(accel_raw[1] << 8 | accel_raw[0]);
    int16_t ay = (int16_t)(accel_raw[3] << 8 | accel_raw[2]);
    int16_t az = (int16_t)(accel_raw[5] << 8 | accel_raw[4]);

    out_data->gyro_x  = gx * GYRO_SENSITIVITY / 1000.0f;
    out_data->gyro_y  = gy * GYRO_SENSITIVITY / 1000.0f;
    out_data->gyro_z  = gz * GYRO_SENSITIVITY / 1000.0f;

    out_data->accel_x = ax * ACCEL_SENSITIVITY / 1000.0f;
    out_data->accel_y = ay * ACCEL_SENSITIVITY / 1000.0f;
    out_data->accel_z = az * ACCEL_SENSITIVITY / 1000.0f;

    return 1;
}