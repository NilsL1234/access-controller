#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void Uart_Init(void);

void Usart_SetRx(uint8_t rx);

uint8_t Usart_GetRx(void);


void Uart_SendByte(uint8_t byte);

void Uart_SendUint32(uint32_t value);

void Uart_SendString(const char *text);

uint8_t Uart_ReceiveByte(void);

// RX Ring buffer (16 byte)

bool Uart_ReadRXBuffer(uint8_t *value);

bool Uart_WriteRXBuffer(uint8_t value);


#endif