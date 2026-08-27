#ifndef BUTTON_H
#define BUTTON_H

# include "stm32f4xx_hal.h"

GPIO_PinState GetCurrentPinState(void);

void SetCurrentPinState(GPIO_PinState currentPinState);

void SetStartTimeDebounce(void);

uint32_t GetStartTimeDebounce(void);

void IncreaseInterruptCounter(void);

#endif

