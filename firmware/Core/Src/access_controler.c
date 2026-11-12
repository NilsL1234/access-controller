# include "access_controler.h"
# include "rfid.h"
# include "sensors.h"
# include "servo.h"
# include "main.h"
# include "oled.h"
# include <stdint.h>
# include <stdbool.h>


// Variables

static ACCESS_CONTROLER_State CurrentAccessControlerState = UNINITIALIZED;
static uint8_t CurrentBlockCounter = 0U;
static const uint8_t MaxBlockCounter = 3U;
static uint32_t BlockStartTick = 0U;
static const uint32_t InitBlockInverval = 10000U;
// amount of seconds the door is locked (doubles every time MaxBlockCounter is reached)
static uint32_t CurrentBlockInverval = InitBlockInverval;

//block counter message display time
static const uint32_t BlockCounterDisplayTime = 300U;
static uint32_t BlockCounterDisplayStartTick = 0U;
static bool BlockCounterMessageDisplayed = false;

// calibrate for convinant use
static const uint32_t UIDValidationInterval = 100U;
static uint32_t UIDValidationStartTick = 0U; 

 
//Functions

// Getter
ACCESS_CONTROLER_State ACCESS_CONTROLER_GetCurrentAccessControlerState(void)
{
    return CurrentAccessControlerState;
}

uint8_t ACCESS_CONTROLER_GetCurrentBlockCounter(void)
{
    return CurrentBlockCounter;
}

uint32_t ACCESS_CONTROLER_GetBlockCounterDisplayTime(void)
{
    return BlockCounterDisplayTime;
}

uint32_t ACCESS_CONTROLER_GetBlockCounterDisplayStartTick(void)
{
    return BlockCounterDisplayStartTick;
}

void ACCESS_CONTROLER_SetBlockCounterMessageDisplayed(bool Displayed)
{
    BlockCounterMessageDisplayed = Displayed;
}

bool ACCESS_CONTROLER_GetBlockCounterMessageDisplayed(void)
{
    return BlockCounterMessageDisplayed;
}

/**
//TODO: finish comment
* @brief
* This function updates the state of the access controler
  and handles necessary transition steps:   - updating CurrentAccessControlerState
                                            - SERVO_UnlockDoor()
                                            - SERVO_LockDoor()
                                            - SERVO_StartLockProcessTimer()
                                            - SERVO_StartUnlockedClosedTimer()
*/
static void ChangeState(ACCESS_CONTROLER_State NewState)
{
    switch (NewState) 
    {
        case LOCKED_READY:
            if(CurrentAccessControlerState == LOCKED_BLOCKED)
            {
                CurrentBlockInverval *= 2;
            }
            CurrentAccessControlerState = LOCKED_READY;
            UIDValidationStartTick = HAL_GetTick();
            break;

        case UNLOCKING:
            if(CurrentAccessControlerState == LOCKED_READY)
            {
                // reset BlockCounter/BlockTime
                CurrentBlockCounter = 0U;
                CurrentBlockInverval = InitBlockInverval;
            }
            
            SERVO_UnlockDoor();
            SERVO_StartLockProcessTimer();
            CurrentAccessControlerState = UNLOCKING;
            break;

        case UNLOCKED_DOOR_CLOSED:
            if(CurrentAccessControlerState == UNLOCKING)
            {
                SERVO_StartUnlockedClosedTimer();
                CurrentAccessControlerState = UNLOCKED_DOOR_CLOSED;
                break;
            }
            else {
                break;
            }
            break;

        case UNLOCKED_DOOR_OPEN:
            switch (CurrentAccessControlerState) 
            {
                case UNINITIALIZED:
                    // TODO: implement alarm sound -> door should be closed and locked on init
                    CurrentAccessControlerState = UNLOCKED_DOOR_OPEN;
                    break;
                case UNLOCKED_DOOR_CLOSED:
                    // maybe here a timer alarm as well 
                    CurrentAccessControlerState = UNLOCKED_DOOR_OPEN;
                    break;
                default: break;
            }
            break;

        case LOCKING:
            if((CurrentAccessControlerState == UNINITIALIZED) ||
            (CurrentAccessControlerState == UNLOCKED_DOOR_OPEN) ||
            (CurrentAccessControlerState == UNLOCKED_DOOR_CLOSED))
            {
                SERVO_LockDoor();
                SERVO_StartLockProcessTimer();
                CurrentAccessControlerState = LOCKING;
                break;
            }
            else
            {
            break;
            }
            break;

        case LOCKED_BLOCKED:
            if(CurrentAccessControlerState == LOCKED_READY)
            {
                CurrentBlockCounter = 0U;
                BlockStartTick = HAL_GetTick();
                CurrentAccessControlerState = LOCKED_BLOCKED;
                break;
            }
            else {
                // UART error could be added
                break;
            }
            default: break;

    }
    SSD1306_DisplayCurrentState();
}

// todo error handling
ACCESS_CONTROLER_Status ACCESS_CONTROLER_InitRFID(void)
{
//todo: error handling
MFRC522Status Status = MFRC522_ExecuteCommand(SOFT_RESET_COMMAND);
Status = MFRC522_InitTransceive();

return ACCESS_CONTROLER_OK;

}

/*
ACCESS_CONTROLER_InitState:
- collects state of bolt/door sensors
- sets access controler state accordingly 
*/
ACCESS_CONTROLER_Status ACCESS_CONTROLER_InitState(void)
{
//todo: error handling sensors
SENSORS_UpdateDoorState();
SENSORS_UpdateLockState();

if((SENSORS_GetBoltState()== BOLT_LOCKED) && (SENSORS_GetDoorState() == DOOR_CLOSED))
{
    ChangeState(LOCKED_READY);
    return ACCESS_CONTROLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_UNLOCKED) && (SENSORS_GetDoorState() == DOOR_CLOSED)) {
    
    ChangeState(LOCKING);
    return ACCESS_CONTROLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_LOCKED) && (SENSORS_GetDoorState() == DOOR_OPEN)) {
    
    ChangeState(UNLOCKING);
    return ACCESS_CONTROLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_UNLOCKED) && (SENSORS_GetDoorState() == DOOR_OPEN)) {

    ChangeState(UNLOCKED_DOOR_OPEN);
    return ACCESS_CONTROLER_OK;
}
//TODO: HANDLE BETWEEN AND INVALID!
else if ((SENSORS_GetBoltState()== BOLT_BETWEEN) && (SENSORS_GetDoorState() == DOOR_OPEN)) {
    ChangeState(UNLOCKING);
    return ACCESS_CONTROLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_BETWEEN) && (SENSORS_GetDoorState() == DOOR_CLOSED)) {
    ChangeState(LOCKING);
    return ACCESS_CONTROLER_OK;
}
else {
return ACCESS_CONTROLER_INITIALIZING_STATE;
}
}

/*
ACCESS_CONTROLER_Run:
- collects state of bolt/door sensors and RFID reader
- updates the access controler state accordingly 
*/
ACCESS_CONTROLER_Status ACCESS_CONTROLER_Run(void)
{

    //todo: error handling sensors
    SENSORS_UpdateDoorState();
    SENSORS_UpdateLockState();

    switch (CurrentAccessControlerState) {

        case LOCKED_READY:
        // check blockcounter 
        if(CurrentBlockCounter >= MaxBlockCounter)
        {
            ChangeState(LOCKED_BLOCKED);
            return ACCESS_CONTROLER_OK;
        }
        if(CurrentBlockCounter > 0U)
        {
            // reader needs to be blocked for 500 ms and displayed, then "ready" again
            SSD1306_DisplayTryBlocked();
        }
        else {
        
        }

        if((HAL_GetTick() - UIDValidationStartTick) >= UIDValidationInterval)
        {
            // check for UID -> unlocking
            UID DetectedUID = {.UIDLength = 0U};
            MFRC522Status MFRC522StatusDetection = MFRC522_DetectUID(&DetectedUID);

            //TODO: think about MFRC522Status, has also MFRC522_UID_VALID so MFRC522_OK is not clear
            if(MFRC522StatusDetection != MFRC522_OK)
            {
            
            }
            bool UIDIsValid = false;

            MFRC522Status MFRC522StatusValidation =  MFRC522_ValidateUID(&DetectedUID, DetectedUID.UIDLength, &UIDIsValid);

            // todo: MFRC522StatusValidation errorhandling

            if(UIDIsValid)
            {
                ChangeState(UNLOCKING);
                return ACCESS_CONTROLER_OK;
            }
            else if (DetectedUID.UIDLength == 0U) {
            // no RFID detected
            UIDValidationStartTick = HAL_GetTick();
            return ACCESS_CONTROLER_OK;
            }
            else {
            // invalid UID detected
            ++ CurrentBlockCounter;
            UIDValidationStartTick = HAL_GetTick();
            BlockCounterDisplayStartTick = HAL_GetTick();
            BlockCounterMessageDisplayed = false;
            return ACCESS_CONTROLER_OK;
            }
        }
        else {
            return ACCESS_CONTROLER_OK;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////

        case UNLOCKING:

        if(SENSORS_GetBoltState() == BOLT_UNLOCKED)
        {
            ChangeState(UNLOCKED_DOOR_CLOSED);
            return ACCESS_CONTROLER_OK;
        }
        else {
            if(SERVO_CheckLockProcessTimerReached())
            {
                SERVO_StopServo();
                // todo: better enum
                return ACCESS_CONTROLER_ERROR;
            }
            else{
                return ACCESS_CONTROLER_OK;
            }
        }

        case UNLOCKED_DOOR_CLOSED:

        if(SENSORS_GetDoorState() == DOOR_OPEN)
        {
            ChangeState(UNLOCKED_DOOR_OPEN);
            return ACCESS_CONTROLER_OK;
        }
        else if(SERVO_CheckUnlockedClosedTimerReached()) 
        {
            ChangeState(LOCKING);
            return ACCESS_CONTROLER_OK;
        }
        else {
        return ACCESS_CONTROLER_OK;
        }

        case UNLOCKED_DOOR_OPEN:

        //when door closed -> locking
        if(SENSORS_GetDoorState() == DOOR_CLOSED)
        {
            ChangeState(LOCKING);
            return ACCESS_CONTROLER_OK;
        }
        else {
        return ACCESS_CONTROLER_OK;
        }

        case LOCKING:

        if(SENSORS_GetBoltState() == BOLT_LOCKED)
        {
            ChangeState(LOCKED_READY);
            return ACCESS_CONTROLER_OK;
        }
        else {

            if(SERVO_CheckLockProcessTimerReached() == true)
            {
                SERVO_StopServo();
                // todo: better enum
                return ACCESS_CONTROLER_ERROR;
            }
            else{
                return ACCESS_CONTROLER_OK;
            }
        }
        
        case LOCKED_BLOCKED:

        // check if block time is reached  
        if((HAL_GetTick() - BlockStartTick) >= CurrentBlockInverval)
        {
            ChangeState(LOCKED_READY);
            return ACCESS_CONTROLER_OK;
        }
        else {
        return ACCESS_CONTROLER_OK;
        }

        case UNINITIALIZED:
        return ACCESS_CONTROLER_INIT_FAIL;

        default: 
        return ACCESS_CONTROLER_STATE_ERROR;
    
    }
}