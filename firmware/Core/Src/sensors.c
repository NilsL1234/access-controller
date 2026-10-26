#include "sensors.h"
#include "uart.h"
#include "main.h"
#include <stdbool.h>


static const uint32_t DEBOUNCE_TIME_INTERVAL_BOLT = 20U;
static const uint32_t DEBOUNCE_TIME_INTERVAL_DOOR = 500U;

static uint32_t StartTimeDebounceBolt = 0U;
static uint32_t StartTimeDebounceDoor = 0U;

static BoltState AcceptedBoltState = BOLT_INIT;
static BoltState PreviousBoltState = BOLT_INIT;
static BoltState CurrentBoltState = BOLT_INIT;

static DoorState AcceptedDoorState = DOOR_INIT;
static DoorState PreviousDoorState = DOOR_INIT;
static DoorState CurrentDoorState = DOOR_INIT;



BoltState SENSORS_GetBoltState(void)
{
    return AcceptedBoltState;
}

DoorState SENSORS_GetDoorState(void)
{
    return AcceptedDoorState;
}

/*
    SENSORS_DetermineCurrentBoltState:
    - get input state for both bolt sensors
    - determine bolt position from them 
*/
static BoltState SENSORS_DetermineCurrentBoltState(void)
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

/*
DetermineCurrentDoorState:
- check pin state from door sensor
- return matching DoorState
*/
static DoorState SENSORS_DetermineCurrentDoorState(void)
{
    if((DoorSensor_GPIO_Port->IDR & DoorSensor_Pin) == 0U)
    {
        return DOOR_SENSOR_ACTIVE;
    }
    else {
        return DOOR_SENSOR_INACTIVE;
    }
}

/*
    UpdateLockState:
    - get current bolt state
    - start timer when different to previous check
    - when new position is stable for +20ms update AcceptedBoltState
*/
bool SENSORS_UpdateLockState(void)
{

    CurrentBoltState = SENSORS_DetermineCurrentBoltState();

    if(CurrentBoltState != PreviousBoltState)
    {
        StartTimeDebounceBolt = HAL_GetTick();
    }

    // update accepted BoltState when its stable
    if(((HAL_GetTick() - StartTimeDebounceBolt) >= DEBOUNCE_TIME_INTERVAL_BOLT) && (CurrentBoltState != AcceptedBoltState))
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

/*
    UpdateDoorState:
    - get current door state from GPIO
    - start timer when different to previous check
    - when new position is stable for +20ms update AcceptedDoorState
*/
bool SENSORS_UpdateDoorState(void)
{

    CurrentDoorState = SENSORS_DetermineCurrentDoorState();

    if(CurrentDoorState != PreviousDoorState)
    {
        StartTimeDebounceDoor = HAL_GetTick();
    }

    // update accepted DoorState when its stable
    if(((HAL_GetTick() - StartTimeDebounceDoor) >= DEBOUNCE_TIME_INTERVAL_DOOR) && (CurrentDoorState != AcceptedDoorState))
    {

    AcceptedDoorState = CurrentDoorState;
    
    if(AcceptedDoorState == DOOR_INIT)
    {
        Uart_SendString("Door state: Init \r\n");
    }
    else if (AcceptedDoorState == DOOR_CLOSED) {
        Uart_SendString("Door state: Closed \r\n");
    }
    else if (AcceptedDoorState == DOOR_OPEN) {
        Uart_SendString("Door state: Open \r\n");
    }
    else{
        Uart_SendString("Door state: Invalid \r\n");
    }
    }

    PreviousDoorState = CurrentDoorState;

    if(AcceptedDoorState !=DOOR_INVALID)
    {
        return true;
    }
    else {
    return false;
    }
}