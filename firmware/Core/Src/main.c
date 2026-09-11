/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "button.h"
#include "uart.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  // PWM 
  // Servo config
  
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);


  //calibration servo

  TIM2->CCR3 = 1750;

  HAL_Delay(1000);

  TIM2->CCR3 = 700;

  HAL_Delay(1000);

  TIM2->CCR3 = 1750;






  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED2);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


//Debounce

const uint32_t DEBOUNCE_TIME_INTERVAL = 20U;
const uint32_t LED2_TIME_INTERVAL = 2000U;

GPIO_PinState AcceptedPinState = GPIO_PIN_SET; 

bool LED2On = false;



//uint32_t StartTimeDebounce = 0U;
uint32_t StartTimeLed2On = 0U;

uint32_t Counter = 0U;

// HAL_GetTick() gives me current time

// Push of button starts LED (for 2 seconds)


// 

Uart_Init();

//Uart_SendByte(65U);

const char *newLine= "\r\n";

const char *textStart = "Start:\r\n";
Uart_SendString(textStart);

const char *textNumber1 = "4.294.967.295:\r\n";
Uart_SendString(textNumber1);
uint32_t number1 = 4294967295U;
Uart_SendUint32(number1);
Uart_SendString(newLine);

// enable interrupt for RXNE (receive not empty)
USART2->CR1 |= USART_CR1_RXNEIE_Msk;
NVIC_EnableIRQ(USART2_IRQn);


//test Uart with buffer

uint8_t a = 0x41;
uint8_t b = 0x42;
uint8_t c = 0x43;
uint8_t d = 0x44;
uint8_t e = 0x45;
uint8_t f = 0x46;



Uart_WriteRXBuffer(a);
Uart_WriteRXBuffer(b);
Uart_WriteRXBuffer(c);
Uart_WriteRXBuffer(d);
Uart_WriteRXBuffer(e);
Uart_WriteRXBuffer(f);

Uart_WriteRXBuffer(a);
Uart_WriteRXBuffer(b);
Uart_WriteRXBuffer(c);
Uart_WriteRXBuffer(d);
Uart_WriteRXBuffer(e);
Uart_WriteRXBuffer(f);

Uart_WriteRXBuffer(a);
Uart_WriteRXBuffer(b);
Uart_WriteRXBuffer(c);
Uart_WriteRXBuffer(d); // last value
Uart_WriteRXBuffer(e);
Uart_WriteRXBuffer(f);




while(1)
{

  uint8_t receivedRXValue;

  if(Uart_ReadRXBuffer(&receivedRXValue))
  {
    Uart_SendByte(receivedRXValue);
  }
}



  while (1)
  {
    // GPIO debounce:

    // deactivate interrupt to avoid race condition / critical section
    // todo: safe both time and state to have shorter crit sec would be cleaner
    NVIC->ICER[0] = 1U << EXTI0_IRQn;

    // new pin state B0 accepted 
    if(((HAL_GetTick() - GetStartTimeDebounce()) >= DEBOUNCE_TIME_INTERVAL) && (GetCurrentPinState() != AcceptedPinState)) 
    {
      AcceptedPinState = GetCurrentPinState();

      if (AcceptedPinState == GPIO_PIN_RESET)
        {
          ++Counter;

          Uart_SendString("countervalue:\r\n");
          Uart_SendUint32(Counter);
          Uart_SendString(newLine);

          if(!LED2On)
          {
            GPIOA->BSRR = (uint32_t)LED2_PIN;
            StartTimeLed2On = HAL_GetTick();
            LED2On = true;
          }        
        } 
    }

    // Activate interrupt again
    NVIC->ISER[0] = 1U << EXTI0_IRQn;

    // turn of led after time interval
    if(((HAL_GetTick() - StartTimeLed2On) >= LED2_TIME_INTERVAL) && (LED2On == true))
      {
        GPIOA->BSRR = (uint32_t)LED2_PIN << 16U;
        LED2On = false;
      }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  
  // enable the clock for SYSCFG (hangs on APB2) | is done by cube already
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  // Clean EXTI0 and set it for port B (EXTICR = External Interrupt Configuration Register)
  SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0_Msk);
  SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB;

  // set Falling and Rising Trigger Selection Register for EXTI0
  EXTI->FTSR |= EXTI_FTSR_TR0_Msk;
  EXTI->RTSR |= EXTI_RTSR_TR0_Msk;

  // set Interrupt Mask Register so the trigger can be used as interrupt 
  EXTI->IMR |= EXTI_IMR_MR0_Msk;

  // enable the IRQ in NVIC 
  NVIC->ISER[0] = 1U << EXTI0_IRQn;

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
