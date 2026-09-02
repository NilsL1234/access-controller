#include "uart.h"
#include <stddef.h>
#include "stm32f4xx_hal.h"


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