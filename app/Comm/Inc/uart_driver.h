#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "usart.h"

void UART_Driver_Init(void);
HAL_StatusTypeDef UART_Driver_Send(uint8_t *data, uint16_t size);
void UART_Driver_StartReceive(void);
uint8_t UART_Driver_IsLinkAlive(void);

#endif