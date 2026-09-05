#ifndef MOTION_VALIDATOR_H
#define MOTION_VALIDATOR_H

#include "encoders.h"
#include "imu_lsm6ds3.h"

typedef enum {
    MOTION_OK = 0,
    MOTION_STALL_DETECTED,     // commanded motion, but encoders show no change
    MOTION_UNEXPECTED,         // no command, but motion detected
    MOTION_SENSOR_MISMATCH,    // encoder and IMU disagree significantly
    MOTION_SENSORS_UNAVAILABLE // can't validate — inputs missing
} MotionStatus;

void MotionValidator_Init(void);
MotionStatus MotionValidator_Check(uint8_t motion_commanded);

#endif