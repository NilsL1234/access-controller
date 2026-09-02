#ifndef UART_H
#define UART_H

#include <stdint.h>

void Uart_Init(void);

void Uart_SendByte(uint8_t byte);

uint8_t Uart_ReceiveByte(void);

void Uart_SendUint32(uint32_t value);

void Uart_SendString(const char *text);

#endif