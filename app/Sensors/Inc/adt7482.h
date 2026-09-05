#ifndef ADT7482_H
#define ADT7482_H

#include "i2c_driver.h"

#define ADT7482_I2C_ADDR   (0x4C << 1)

typedef struct {
    float local_temp_c;
    float remote_temp_c;
} ADT7482_Data;

uint8_t ADT7482_Init(void);
uint8_t ADT7482_IsAvailable(void);
uint8_t ADT7482_Read(ADT7482_Data *out_data);

#endif