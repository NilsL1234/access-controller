#include "uart.h"
#include <stdbool.h>
#include <stddef.h>
#include "stm32f4xx_hal.h"


volatile uint8_t uart_rx_value = 0U;
volatile uint8_t rx_buffer [16] = {0};

volatile uint8_t rx_buffer_pos_read = 0;
volatile uint8_t rx_buffer_pos_write = 0;

volatile bool rx_buffer_full = false;
volatile bool rx_buffer_empty = true;


  // cases:

    /*
    - writer == reader:
        - which state?
        - full? -> only reading
        - empty? -> only writing

    - writer != reader: reading/writing possible
    
    
    - interrupt: buffer is full but writing interrupts reading
    
    
    */

bool Uart_WriteRXBuffer(uint8_t value)
{
    if(rx_buffer_full)
    {
        return false;
    }

    rx_buffer[rx_buffer_pos_write] = value;
    ++rx_buffer_pos_write;
    rx_buffer_empty = false;

    if(rx_buffer_pos_write == (sizeof(rx_buffer)/sizeof(rx_buffer[0])))
    {
        rx_buffer_pos_write = 0;
    }

    if(rx_buffer_pos_write == rx_buffer_pos_read)
    {
        rx_buffer_full = true;
    }
    return true;
}

bool Uart_ReadRXBuffer(uint8_t *value)
{

    if(rx_buffer_empty)
    {
        return false;
    }

    uint32_t irqEnabled = NVIC_GetEnableIRQ(USART2_IRQn);
    NVIC_DisableIRQ(USART2_IRQn);

    *value = rx_buffer[rx_buffer_pos_read];
    ++rx_buffer_pos_read;
    rx_buffer_full = false;

    if(rx_buffer_pos_read == (sizeof(rx_buffer)/sizeof(rx_buffer[0])))
    {
        rx_buffer_pos_read = 0;
    }

    if(rx_buffer_pos_read == rx_buffer_pos_write)
    {
        rx_buffer_empty = true;
    }

    if(irqEnabled != 0U)
    {
        NVIC_EnableIRQ(USART2_IRQn);
    }

    return true;    
}


void Uart_SetRx(uint8_t rx)
{
    uart_rx_value = rx;
}

uint8_t Uart_GetRx(void)
{
    return uart_rx_value;
}

void Uart_Init(void)
{
    /*
     - Pins for USART2: - PA2 (USART2_TX)
                        - PA3 (USART2_RX)
                        - PA4 (USART2_CK)
     - Bus: APB1 (max.45 MHz)
     */
     

     // init PA2
     GPIOA->MODER &= ~(GPIO_MODER_MODER2_Msk);
     GPIOA->MODER |= GPIO_MODER_MODER2_1;
     GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED2);
     GPIOA->PUPDR |= GPIO_PUPDR_PUPD2_0;
     GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk);
     GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL2_0 | GPIO_AFRL_AFSEL2_1 | GPIO_AFRL_AFSEL2_2);

     // init PA3
     GPIOA->MODER &= ~(GPIO_MODER_MODER3_Msk);
     GPIOA->MODER |= GPIO_MODER_MODER3_1;
     GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED3);
     GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;
     GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk);
     GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_2);

     // activate clock for USART2 (APB1->PCLK1 ->42mHz)
     RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // config USART2 register:

    // enable
    USART2->CR1 |= USART_CR1_UE;
    
    // word length to 8 bit (reset)
    USART2->CR1 &= ~(USART_CR1_M_Msk);

    // set number of stop bits to 1 (reset)
    USART2->CR2 &= ~(USART_CR2_STOP_Msk);

    // calculate baudrate for USART:

    // Baudrate:
    // APB1: 42mHz (42 000 000) 
    // oversampling 16
    // Baudrate aim: 115200
    // Baud with oversampling: 115 200 * 16 = 1 843 200
    // clock: 42000000 / 1843200 = 22,7864583333

    uint32_t PCLK1 = HAL_RCC_GetPCLK1Freq(); // 42 000 000 
    uint32_t OversamplingValue = 16;
    uint32_t Baudrate = 115200;
    uint32_t BaudrateOversampling = Baudrate * OversamplingValue;
    
    uint32_t USART_DivMantissa = PCLK1 / BaudrateOversampling;

    uint32_t USART_DivFractionNumerator = (PCLK1 % BaudrateOversampling) << 4U;
    uint32_t USART_DivFraction = (USART_DivFractionNumerator + BaudrateOversampling / 2U) / BaudrateOversampling;

    if(USART_DivFraction == 16U)
    {
      ++USART_DivMantissa;
      USART_DivFraction = 0U;

    }

    // reset brr (baudrate-register)
    USART2->BRR &= ~(USART_BRR_DIV_Mantissa_Msk);
    USART2->BRR &= ~(USART_BRR_DIV_Fraction_Msk);
        
    // set brr
    USART2->BRR |= USART_DivMantissa << USART_BRR_DIV_Mantissa_Pos;
    USART2->BRR |= USART_DivFraction << USART_BRR_DIV_Fraction_Pos;

    // transmitte enable (sends idle frame at first)
    USART2->CR1 |= USART_CR1_TE;

    // receive enable
    USART2->CR1 |= USART_CR1_RE;

}

void Uart_SendByte(uint8_t byte)
{
  while((USART2->SR & USART_SR_TXE_Msk) == 0U)
  {
  }
    USART2->DR = byte;
}

uint8_t Uart_ReceiveByte(void)
{
    while((USART2->SR & USART_SR_RXNE_Msk) == 0U)
    {
    }
    return USART2->DR; 
}

void Uart_SendUint32(uint32_t value)
{
    if(value==0U)
    { 
        Uart_SendByte('0');
        return;
    }
    
    char result[11];

    size_t i = 0;
    size_t lengthArray = 11;
    size_t digitsToSend = 0;


    while(i<lengthArray)
    {
        result[i] = value % 10;
        ++digitsToSend;
        if((value / 10) == 0U)
        {
            break;
        }
        else {
        value = value / 10;
        ++i;
        }
         
    }

    i = digitsToSend;
    
    while(i > 0U)
    {
        Uart_SendByte('0'+ result[i-1]);
        --i;
    }

}

void Uart_SendString(const char *text)
{
  size_t i = 0U;
  char currentChar = text[i];

  while(currentChar != '\0')
  {
    Uart_SendByte(currentChar);
    ++i;
    currentChar = text[i];
  }
}