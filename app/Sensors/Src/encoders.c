#include "encoders.h"

// Maps each logical wheel to its physical timer handle.
// Update these pointers once real timer assignments are confirmed.
static TIM_HandleTypeDef *encoder_timers[ENCODER_COUNT] = {
    &htim1, // ENCODER_FRONT_LEFT
    &htim2, // ENCODER_FRONT_RIGHT
    &htim3, // ENCODER_REAR_LEFT
    &htim4, // ENCODER_REAR_RIGHT
};

static int32_t last_count[ENCODER_COUNT];

void Encoders_Init(void)
{
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();

    for (int i = 0; i < ENCODER_COUNT; i++)
    {
        HAL_TIM_Encoder_Start(encoder_timers[i], TIM_CHANNEL_ALL);
        last_count[i] = (int32_t)__HAL_TIM_GET_COUNTER(encoder_timers[i]);
    }
}

int32_t Encoders_ReadCount(EncoderId id)
{
    if (id >= ENCODER_COUNT) {
        return 0; // invalid id — caller error, not a hardware fault
    }
    return (int32_t)__HAL_TIM_GET_COUNTER(encoder_timers[id]);
}

int32_t Encoders_ReadDelta(EncoderId id)
{
    if (id >= ENCODER_COUNT) {
        return 0;
    }

    int32_t current = Encoders_ReadCount(id);
    int32_t delta = current - last_count[id];
    last_count[id] = current;

    return delta;
}