#ifndef MLX90614_H
#define MLX90614_H

#include "i2c_driver.h"

#define MLX90614_I2C_ADDR   (0x5A << 1)

typedef struct {
    float ambient_temp_c;
    float object_temp_c;
} MLX90614_Data;

uint8_t MLX90614_Init(void);
uint8_t MLX90614_IsAvailable(void);
uint8_t MLX90614_Read(MLX90614_Data *out_data);

#endif