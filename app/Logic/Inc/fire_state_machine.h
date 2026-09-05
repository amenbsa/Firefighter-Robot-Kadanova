#ifndef FIRE_STATE_MACHINE_H
#define FIRE_STATE_MACHINE_H

#include "adt7482.h"
#include "mlx90614.h"
#include "mq2.h"

typedef enum {
    FIRE_STATE_IDLE = 0,
    FIRE_STATE_WARNING,
    FIRE_STATE_CONFIRMED,
    FIRE_STATE_SENSOR_FAULT
} FireState;

void FireStateMachine_Init(void);
FireState FireStateMachine_Update(void);

#endif