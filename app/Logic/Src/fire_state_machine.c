#include "fire_state_machine.h"

// Placeholder thresholds — NOT calibrated against real sensors or fire
// scenarios. Must be validated with real hardware/testing before trusting
// this for any actual safety decision.
#define TEMP_WARNING_C      50.0f
#define TEMP_CONFIRM_C      80.0f
#define GAS_WARNING_PPM      300.0f
#define GAS_CONFIRM_PPM      1000.0f

static FireState current_state = FIRE_STATE_IDLE;

void FireStateMachine_Init(void)
{
    current_state = FIRE_STATE_IDLE;
}

FireState FireStateMachine_Update(void)
{
    uint8_t adt_ok = ADT7482_IsAvailable();
    uint8_t mlx_ok = MLX90614_IsAvailable();
    uint8_t mq2_ok = MQ2_IsAvailable();

    // If ALL fire-relevant sensors are down, we genuinely can't assess —
    // this is different from "no fire detected" and must be surfaced
    // distinctly so upstream logic doesn't treat silence as safety.
    if (!adt_ok && !mlx_ok && !mq2_ok) {
        current_state = FIRE_STATE_SENSOR_FAULT;
        return current_state;
    }

    uint8_t warning_indicators = 0;
    uint8_t confirm_indicators = 0;

    if (mlx_ok) {
        MLX90614_Data mlx_data;
        if (MLX90614_Read(&mlx_data)) {
            if (mlx_data.object_temp_c >= TEMP_CONFIRM_C) confirm_indicators++;
            else if (mlx_data.object_temp_c >= TEMP_WARNING_C) warning_indicators++;
        }
    }

    if (adt_ok) {
        ADT7482_Data adt_data;
        if (ADT7482_Read(&adt_data)) {
            if (adt_data.remote_temp_c >= TEMP_CONFIRM_C) confirm_indicators++;
            else if (adt_data.remote_temp_c >= TEMP_WARNING_C) warning_indicators++;
        }
    }

    if (mq2_ok) {
        float ppm;
        if (MQ2_ReadPPM(&ppm)) {
            if (ppm >= GAS_CONFIRM_PPM) confirm_indicators++;
            else if (ppm >= GAS_WARNING_PPM) warning_indicators++;
        }
    }

    // Require at least 2 independent confirming indicators before escalating
    // to CONFIRMED, given several upstream sensors are still unverified/placeholder.
    if (confirm_indicators >= 2) {
        current_state = FIRE_STATE_CONFIRMED;
    } else if (confirm_indicators >= 1 || warning_indicators >= 1) {
        current_state = FIRE_STATE_WARNING;
    } else {
        current_state = FIRE_STATE_IDLE;
    }

    return current_state;
}