#ifndef ENCODERS_H
#define ENCODERS_H

#include "tim.h"

typedef enum {
    ENCODER_FRONT_LEFT = 0,
    ENCODER_FRONT_RIGHT,
    ENCODER_REAR_LEFT,
    ENCODER_REAR_RIGHT,
    ENCODER_COUNT   // total number of encoders — used for array sizing/loops
} EncoderId;

void Encoders_Init(void);
int32_t Encoders_ReadCount(EncoderId id);
int32_t Encoders_ReadDelta(EncoderId id);

#endif