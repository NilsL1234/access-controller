#include "sensors.h"
#include "uart.h"
#include "main.h"
#include <stdbool.h>


static const uint32_t DEBOUNCE_TIME_INTERVAL = 20U;
static uint32_t StartTimeDebounce = 0U;

static BoltState AcceptedBoltState = BOLT_INIT;
static BoltState PreviousBoltState = BOLT_INIT;
static BoltState CurrentBoltState = BOLT_INIT;


BoltState GetBoltState(void)
{
    return AcceptedBoltState;
}

static BoltState DetermineCurrentBoltState(void)
{
    BoltSensorState locked = BOLT_SENSOR_INIT;
    BoltSensorState unlocked = BOLT_SENSOR_INIT;

    if((BoltSensorLocked_GPIO_Port->IDR & BoltSensorLocked_Pin) == 0U)
    {
        locked = BOLT_SENSOR_ACTIVE;
    }
    else {
        locked = BOLT_SENSOR_INACTIVE;
    }

    if((BoltSensorUnlocked_GPIO_Port->IDR & BoltSensorUnlocked_Pin) == 0U)
    {
        unlocked = BOLT_SENSOR_ACTIVE;
    }
    else {
        unlocked = BOLT_SENSOR_INACTIVE;
    }

    if((locked == BOLT_SENSOR_ACTIVE) && (unlocked == BOLT_SENSOR_ACTIVE))
    {
        return BOLT_INVALID;
    }
    else if ((locked == BOLT_SENSOR_ACTIVE) && (unlocked == BOLT_SENSOR_INACTIVE)) {
        return BOLT_LOCKED;
    }
    else if ((locked == BOLT_SENSOR_INACTIVE) && (unlocked == BOLT_SENSOR_ACTIVE)) {
        return BOLT_UNLOCKED;
    }
    else {
        return BOLT_BETWEEN;
    }
}

bool UpdateLockState(void)
{

    CurrentBoltState = DetermineCurrentBoltState();

    if(CurrentBoltState != PreviousBoltState)
    {
        StartTimeDebounce = HAL_GetTick();
    }

    if(((HAL_GetTick() - StartTimeDebounce) >= DEBOUNCE_TIME_INTERVAL) && (CurrentBoltState != AcceptedBoltState))
    {

    AcceptedBoltState = CurrentBoltState;
    
    if(AcceptedBoltState == BOLT_INIT)
    {
        Uart_SendString("Bolt state: Init \r\n");
    }
    else if (AcceptedBoltState == BOLT_LOCKED) {
        Uart_SendString("Bolt state: Locked \r\n");
    }
    else if (AcceptedBoltState == BOLT_UNLOCKED) {
        Uart_SendString("Bolt state: Unlocked \r\n");
    }
    else if (AcceptedBoltState == BOLT_BETWEEN) {
        Uart_SendString("Bolt state: Between \r\n");
    }
    else{
        Uart_SendString("Bolt state: Invalid \r\n");
    }
    }

    PreviousBoltState = CurrentBoltState;

    if(AcceptedBoltState !=BOLT_INVALID)
    {
        return true;
    }
    else {
    return false;
    }
}


