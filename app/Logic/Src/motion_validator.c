#include "motion_validator.h"
#include <math.h>
#include <stdlib.h>

#define STALL_THRESHOLD_COUNTS   5     // encoder delta below this = "not moving"
#define UNEXPECTED_THRESHOLD     5     // encoder delta above this while uncommanded = unexpected
#define IMU_ACCEL_THRESHOLD_G    0.05f // rough noise floor for "IMU sees motion"

void MotionValidator_Init(void)
{
    // No internal state to initialize yet — encoders/IMU are initialized
    // independently in their own Init() calls.
}

MotionStatus MotionValidator_Check(uint8_t motion_commanded)
{
    if (!IMU_IsAvailable()) {
        // Encoders alone can still validate stall/unexpected motion,
        // just without the IMU cross-check — degrade gracefully rather
        // than refusing entirely.
    }

    int32_t total_encoder_delta = 0;
    for (int i = 0; i < ENCODER_COUNT; i++) {
        total_encoder_delta += labs(Encoders_ReadDelta((EncoderId)i));
    }

    uint8_t encoder_shows_motion = (total_encoder_delta > STALL_THRESHOLD_COUNTS);

    if (motion_commanded && !encoder_shows_motion) {
        return MOTION_STALL_DETECTED;
    }

    if (!motion_commanded && total_encoder_delta > UNEXPECTED_THRESHOLD) {
        return MOTION_UNEXPECTED;
    }

    if (IMU_IsAvailable()) {
        IMU_Data imu_data;
        if (IMU_Read(&imu_data)) {
            float accel_magnitude = sqrtf(
                imu_data.accel_x * imu_data.accel_x +
                imu_data.accel_y * imu_data.accel_y +
                imu_data.accel_z * imu_data.accel_z
            );

            // Very rough cross-check: if encoders show strong motion but IMU
            // shows essentially no acceleration change at all, flag mismatch.
            // NOTE: this comparison is intentionally coarse — a proper check
            // needs a baseline/gravity-subtracted reference, not raw magnitude.
            uint8_t imu_shows_motion = (fabsf(accel_magnitude - 1.0f) > IMU_ACCEL_THRESHOLD_G);

            if (encoder_shows_motion && !imu_shows_motion && total_encoder_delta > (STALL_THRESHOLD_COUNTS * 3)) {
                return MOTION_SENSOR_MISMATCH;
            }
        }
    }

    return MOTION_OK;
}