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
#include "access_controler.h"
#include "button.h"
#include "uart.h"
#include "sensors.h"
#include "rfid.h"


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

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

SPI_HandleTypeDef* GetSPIHandleTypeDef(void)
{
  return &hspi2;
}

TIM_HandleTypeDef* GetTIMHandleTypeDef(void)
{
  return &htim2;
}





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
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED2);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

/*
M7-M8: Timebased Processes/Debounce/Interrupts
  - setup
  - rest in main loop
*/

const uint32_t DEBOUNCE_TIME_INTERVAL = 20U;
const uint32_t LED2_TIME_INTERVAL = 2000U;

GPIO_PinState AcceptedPinState = GPIO_PIN_SET; 

bool LED2On = false;
uint32_t StartTimeLed2On = 0U;
uint32_t Counter = 0U;

/*
M9: UART:
  - send uint/string to terminal
  - receive input as interrupt
  - send input back to display for example in terminal
*/

Uart_Init();

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

// example uart ring buffer
uint8_t a = 0x41;
Uart_WriteRXBuffer(a);

/*
M10: Servo and PWM
  - first calibration of servo for locked and unlocked states
  - NOT FINISHED
*/
  
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

//calibration servo
// 1750 locked
// 1270 unlocked 

/*
TIM2->CCR3 = 1270;
HAL_Delay(1000);

TIM2->CCR3 = 1270;
*/

//TIM2->CCR3 = 1750;
//HAL_Delay(1000);




/*
M12: SPI basics - reading register
- send address of a register via SPI to the MFRC522 to read the content
- read ComIEnReg as example:
- address: 0x02
- expected register content: 0x80
*/

uint8_t ComIEnRegAddress = 0x02;

// assemble byte 
uint8_t Read = 0x80;
uint8_t AddressPos = 1U;
uint8_t ReadComIEnRegByte = (ComIEnRegAddress << AddressPos) | Read;

uint8_t DummyByte = 0x00; 


// enable SPI
SPI2->CR1 |= SPI_CR1_SPE_Msk;

//wait till tx empty is set
while((SPI2->SR & SPI_SR_TXE_Msk) == 0U)
{}
// reset NSS to start spi transfer
RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin << 16U;

// fill DR
SPI2->DR = ReadComIEnRegByte;

// handle received dummy byte
while((SPI2->SR & SPI_SR_RXNE_Msk) == 0U)
{}
(void)SPI2->DR;

//send dummybyte
while((SPI2->SR & SPI_SR_TXE_Msk) == 0U)
{}
SPI2->DR = DummyByte;

// receive ComIEnReg byte
while((SPI2->SR & SPI_SR_RXNE_Msk) == 0U)
{}
uint8_t ComIEnReg = SPI2->DR;

// set NSS to end transfer
RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin;

/*
M12: SPI basics - writing register
- write new value for TxSpeed in TxModeReg register
- TxModeReg: address 12h
- TxSpeed: bit 6 to 4
*/

uint8_t TxModeRegAddress = 0x12;
uint8_t ReadTxModeRegByte = (TxModeRegAddress << AddressPos) | Read;
uint8_t WriteTxModeRegByte = (TxModeRegAddress << AddressPos);



//current value of TxModeReg is 0x00

// set to 0011 0000

uint8_t TxSpeed = 3U;
uint8_t PositionTxSpeed = 4U;
uint8_t mask = 0x70; // 6:4 

//reset register
uint8_t TxModeReg = SPI2->DR;
TxModeReg &= ~(mask);
TxModeReg |= TxSpeed << PositionTxSpeed;

// how to write new value into a register?
// first byte is address from the register and write mode
// second byte is the new value to be written into the register

while((SPI2->SR & SPI_SR_TXE_Msk) == 0U)
{}

RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin << 16U;

// fill DR with writebyte
SPI2->DR = WriteTxModeRegByte;

// handle received dummy byte
while((SPI2->SR & SPI_SR_RXNE_Msk) == 0U)
{}
(void)SPI2->DR;

//send new value for register
while((SPI2->SR & SPI_SR_TXE_Msk) == 0U)
{}
SPI2->DR = TxModeReg;

// handle received dummy byte
while((SPI2->SR & SPI_SR_RXNE_Msk) == 0U)
{}
(void)SPI2->DR;

// set NSS to end transfer
RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin;

//todo: write the reset value into the register and read afterwards again using HAL

// send byte with address and operation
// receive dummy (should be done by hal)
// send new value
// receive dummy

// read register

/*
M12: SPI basics (reading/writing register with HAL)
- read the TxModeRegister with HAL
- expected value: 0x30
- write the register to reset value 0x00 using HAL
- read again to confirm
*/

// SPI transfer variables
const uint8_t SPIsizeTransferData = 2U;
const uint32_t SPItimeoutTransferDurationMS = 10U;


// read TxModeRegister

RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin << 16U;

const uint8_t ReadTxModeRegData[] = {ReadTxModeRegByte, DummyByte};
uint8_t RxDataSPI[2] = {0xff, 0xff};
HAL_SPI_TransmitReceive(&hspi2, ReadTxModeRegData, RxDataSPI, SPIsizeTransferData, SPItimeoutTransferDurationMS);

RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin;


// write TxModeRegister

RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin << 16U;

uint8_t TxModeRegResetValue = 0x00U;
const uint8_t WriteTxModeRegData[] = {WriteTxModeRegByte, TxModeRegResetValue};
HAL_SPI_TransmitReceive(&hspi2, WriteTxModeRegData, RxDataSPI, SPIsizeTransferData, SPItimeoutTransferDurationMS);

RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin;


// read TxModeRegister

RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin << 16U;

HAL_SPI_TransmitReceive(&hspi2, ReadTxModeRegData, RxDataSPI, SPIsizeTransferData, SPItimeoutTransferDurationMS);

RFID_NSS_GPIO_Port->BSRR = (uint32_t)RFID_NSS_Pin;


/*
M13: MFRC522 Driver
- ...
- ...
- wait >5ms after RF is on
*/

// enable interrupt: 

// clean EXTI2 and set it for port C
SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI2_Msk);
SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PC;

// set Falling Trigger Selection Register for EXTI2
EXTI->FTSR |= EXTI_FTSR_TR2_Msk;

// set Interrupt Mask Register so the trigger can be used as interrupt 
EXTI->IMR |= EXTI_IMR_MR2_Msk;

// enable the IRQ in NVIC 
NVIC_EnableIRQ(EXTI2_IRQn);

/*MFRC522_ExecuteCommand(SOFT_RESET_COMMAND);
HAL_Delay(1000);
MFRC522Status UIDStatus = MFRC522_InitTransceive();
HAL_Delay(1000);
*/



ACCESS_CONTROLER_InitRFID();

while (1) {

if(ACCESS_CONTROLER_GetCurrentAccessControlerState() == UNINITIALIZED)
{
  ACCESS_CONTROLER_InitState();
}

ACCESS_CONTROLER_Run();

}

/*
UIDStatus = MFRC522_DetectUID(&TestUID);
bool IsValid = false; 
UIDStatus =  MFRC522_ValidateUID(&TestUID, 4U, &IsValid);
if(IsValid)
{
  uint8_t asd = 1U;
}
*/


while (1)
{

/*
M7-M8: Timebased Processes/Interrupts
  - when button is pushed, LED is on for 2 seconds 
  - push of button is tracked with interrupt
  - race conditions are covered
  - debounce of button push is implemented (after 20ms new button state is accepted)
*/

// deactivate interrupt to avoid race condition / critical section
NVIC->ICER[0] = 1U << EXTI0_IRQn;

// new pin state B0 accepted 
if(((HAL_GetTick() - GetStartTimeDebounce()) >= DEBOUNCE_TIME_INTERVAL) && (GetCurrentPinState() != AcceptedPinState)) 
{
  AcceptedPinState = GetCurrentPinState();

  if(AcceptedPinState == GPIO_PIN_RESET)
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

/*
M9: UART
  - read keyboard input from Uart Ring buffer for new values (get written by interrupt)
  - display 
*/ 
uint8_t receivedRXValue;

if(Uart_ReadRXBuffer(&receivedRXValue))
{
  Uart_SendByte(receivedRXValue);
}

/*
M11: Sensors
  - update the current lock state with both sensors
  - TODO: Lockedsensor->PC0->PinA5
          Unlockedsensor->PC1->PinA4
*/

SENSORS_UpdateLockState();

}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RFID_NSS_GPIO_Port, RFID_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RFID_RST_GPIO_Port, RFID_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BoltSensorLocked_Pin BoltSensorUnlocked_Pin RFID_IRQ_Pin */
  GPIO_InitStruct.Pin = BoltSensorLocked_Pin|BoltSensorUnlocked_Pin|RFID_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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

  /*Configure GPIO pin : RFID_NSS_Pin */
  GPIO_InitStruct.Pin = RFID_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RFID_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RFID_RST_Pin */
  GPIO_InitStruct.Pin = RFID_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RFID_RST_GPIO_Port, &GPIO_InitStruct);

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
