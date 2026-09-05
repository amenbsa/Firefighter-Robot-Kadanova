#include "uart_driver.h"

static uint8_t rx_byte;
static uint32_t last_rx_tick;

void UART_Driver_Init(void)
{
    MX_USART2_UART_Init();
    UART_Driver_StartReceive();
}

HAL_StatusTypeDef UART_Driver_Send(uint8_t *data, uint16_t size)
{
    return HAL_UART_Transmit_DMA(&huart2, data, size);
}

void UART_Driver_StartReceive(void)
{
    HAL_StatusTypeDef status;
    status = HAL_UART_Receive_DMA(&huart2, &rx_byte, 1);
    if (status != HAL_OK)
    {
        /* DMA reception could not be started — IsLinkAlive's timeout
         * will catch this if it persists */
    }
}

uint8_t UART_Driver_IsLinkAlive(void)
{
    if (HAL_GetTick() - last_rx_tick < 500)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        last_rx_tick = HAL_GetTick();
        UART_Driver_StartReceive();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        /* HAL's DMA error handling resets UART/DMA state before this
         * callback runs, so it's safe to re-arm directly here. */
        UART_Driver_StartReceive();
    }
}
