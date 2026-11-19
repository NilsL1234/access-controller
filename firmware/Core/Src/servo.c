#include "servo.h"
#include "sensors.h"
#include "uart.h"
#include "main.h"

// timer for locking/unlocking process 
static uint32_t LockProcessTimer = 0U;
static const uint32_t LockProcessTimerLimit = 500U;

// timer for door unlocked but not opened to lock door again
static uint32_t UnlockedClosedTimer = 0U;
static const uint32_t UnlockedClosedTimerLimit = 30000U;


Servo_Status SERVO_UnlockDoor(void)
{
    if(((SENSORS_GetBoltState() == BOLT_LOCKED) && (SENSORS_GetDoorState()==DOOR_CLOSED))||
       ((SENSORS_GetBoltState() == BOLT_LOCKED) && (SENSORS_GetDoorState()==DOOR_OPEN))||
       ((SENSORS_GetBoltState() == BOLT_BETWEEN) && (SENSORS_GetDoorState()==DOOR_OPEN)) 
    )
    {
        TIM2->CCR3 = 1270;
        return SERVO_OK;
    }
    else{
        return SERVO_ERROR;
    }
}

Servo_Status SERVO_LockDoor(void)
{
    if(((SENSORS_GetBoltState() == BOLT_UNLOCKED) && (SENSORS_GetDoorState()==DOOR_CLOSED))||
       ((SENSORS_GetBoltState() == BOLT_BETWEEN) && (SENSORS_GetDoorState()==DOOR_CLOSED)))
    {
        TIM2->CCR3 = 1750;
        return SERVO_OK;
    }
    else{
        return SERVO_ERROR;
    }
}

void SERVO_StartLockProcessTimer(void)
{
LockProcessTimer = HAL_GetTick();
}

void SERVO_StartUnlockedClosedTimer(void)
{
UnlockedClosedTimer = HAL_GetTick();
}

bool SERVO_CheckLockProcessTimerReached(void)
{
    if((HAL_GetTick()-LockProcessTimer) >= LockProcessTimerLimit)
    {
        return true;
    }
    else {
        return false;
    }
}

bool SERVO_CheckUnlockedClosedTimerReached(void)
{
    if((HAL_GetTick()-UnlockedClosedTimer) >= UnlockedClosedTimerLimit)
    {
        return true;
    }
    else {
        return false;
    }
}

void SERVO_StopServo(void)
{
    HAL_TIM_PWM_Stop(GetTIMHandleTypeDef(), TIM_CHANNEL_3);
}


