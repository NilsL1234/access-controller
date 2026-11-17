#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    UART_OK      = 0x00,
    UART_ERROR   = 0x01,
    UART_TIMEOUT = 0x02
} UART_Status;


void Uart_Init(void);

void Usart_SetRx(uint8_t rx);

uint8_t Usart_GetRx(void);

UART_Status Uart_SendByte(uint8_t byte);

UART_Status Uart_SendUint32(uint32_t value);

UART_Status Uart_SendString(const char *text);

uint8_t Uart_ReceiveByte(void);

// RX Ring buffer (16 byte)

bool Uart_ReadRXBuffer(uint8_t *value);

bool Uart_WriteRXBuffer(uint8_t value);


#endif
