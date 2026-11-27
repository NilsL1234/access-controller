#include "sensors.h"
#include "uart.h"
#include "main.h"
#include "log.h"
#include <stdbool.h>


static const uint32_t DEBOUNCE_TIME_INTERVAL_BOLT = 20U;
static const uint32_t DEBOUNCE_TIME_INTERVAL_DOOR = 50U;

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
        return DOOR_CLOSED;
    }
    else {
        return DOOR_OPEN;
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


    if (AcceptedBoltState == BOLT_LOCKED) {
        LOG_Write(LOG_INFO, "SENSOR", "Bolt locked");
    }
    else if (AcceptedBoltState == BOLT_UNLOCKED) {
        LOG_Write(LOG_INFO, "SENSOR", "Bolt unlocked");
    }
    else if (AcceptedBoltState == BOLT_BETWEEN) {
        LOG_Write(LOG_INFO, "SENSOR", "Bolt between");
    }
    else{
        LOG_Write(LOG_INFO, "SENSOR", "Invalid AcceptedBoltState");
    }
    }

    PreviousBoltState = CurrentBoltState;

    if(AcceptedBoltState != BOLT_INVALID)
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

    if (AcceptedDoorState == DOOR_CLOSED) {
        LOG_Write(LOG_INFO, "SENSOR", "Door closed");
    }
    else if (AcceptedDoorState == DOOR_OPEN) {
        LOG_Write(LOG_INFO, "SENSOR", "Door open");
    }
    else{
        LOG_Write(LOG_INFO, "SENSOR", "Invalid AcceptedDoorState");
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