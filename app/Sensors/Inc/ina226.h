#ifndef INA226_H
#define INA226_H

#include "i2c_driver.h"

#define INA226_I2C_ADDR   (0x40 << 1)

typedef struct {
    float bus_voltage_v;
    float current_a;
    float power_w;
} INA226_Data;

uint8_t INA226_Init(void);
uint8_t INA226_IsAvailable(void);
uint8_t INA226_Read(INA226_Data *out_data);

#endif