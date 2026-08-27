# include "button.h"

static volatile GPIO_PinState CurrentPinState = GPIO_PIN_SET;

static volatile uint32_t StartTimeDebounce = 0U;

static volatile uint32_t InterruptCounter = 0U;


GPIO_PinState GetCurrentPinState(void)
{
    return CurrentPinState;
}

void SetCurrentPinState(GPIO_PinState currentPinState)
{
    CurrentPinState = currentPinState;
}

void SetStartTimeDebounce(void)
{
    StartTimeDebounce = HAL_GetTick();
}

uint32_t GetStartTimeDebounce(void)
{
    return StartTimeDebounce;
}

void IncreaseInterruptCounter(void)
{
  ++InterruptCounter;
}