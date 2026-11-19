#include "log.h"
#include "uart.h"
#include "stm32f4xx_hal.h"


void LOG_Write(LOG_Level Level, const char *Source, const char *Message)
{
    if(Source == NULL || Message == NULL)
    {
        return;
    }

    switch(Level){

    // Bsp: "[10 ms]INFO CTRL: Initialisation complete"
    case LOG_INFO:
        if (Uart_SendString("[") != UART_OK)
        {
            return;
        }
        if (Uart_SendUint32(HAL_GetTick()) != UART_OK)
        {
            return;
        }
        if (Uart_SendString(" ms] INFO ") != UART_OK)
        {
            return;
        }
        if (Uart_SendString(Source) != UART_OK)
        {
            return;
        }
        if (Uart_SendString(": ") != UART_OK)
        {
            return;
        }
        if (Uart_SendString(Message) != UART_OK)
        {
            return;
        }
        if (Uart_SendString("\r\n") != UART_OK)
        {
            return;
        }
        break;

    case LOG_WARN:
        if (Uart_SendString("[") != UART_OK)
        {
            return;
        }
        if (Uart_SendUint32(HAL_GetTick()) != UART_OK)
        {
            return;
        }
        if (Uart_SendString(" ms] WARN ") != UART_OK)
        {
            return;
        }
        if (Uart_SendString(Source) != UART_OK)
        {
            return;
        }
        if (Uart_SendString(": ") != UART_OK)
        {
            return;
        }
        if (Uart_SendString(Message) != UART_OK)
        {
            return;
        }
        if (Uart_SendString("\r\n") != UART_OK)
        {
            return;
        }
        break;

    case LOG_ERROR:
        if (Uart_SendString("[") != UART_OK)
        {
            return;
        }
        if (Uart_SendUint32(HAL_GetTick()) != UART_OK)
        {
            return;
        }
        if (Uart_SendString(" ms] ERROR ") != UART_OK)
        {
            return;
        }
        if (Uart_SendString(Source) != UART_OK)
        {
            return;
        }
        if (Uart_SendString(": ") != UART_OK)
        {
            return;
        }
        if (Uart_SendString(Message) != UART_OK)
        {
            return;
        }
        if (Uart_SendString("\r\n") != UART_OK)
        {
            return;
        }
        break;
    default:
        return;
    }
}