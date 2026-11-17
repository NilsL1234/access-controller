# include "access_controller.h"
# include "rfid.h"
# include "sensors.h"
# include "servo.h"
# include "main.h"
# include "oled.h"
#include "log.h"
# include <stdint.h>
# include <stdbool.h>


// Variables

static ACCESS_CONTROLLER_State CurrentAccessControllerState = UNINITIALIZED;
static uint8_t CurrentBlockCounter = 0U;
static const uint8_t MaxBlockCounter = 3U;
static uint32_t BlockStartTick = 0U;
static const uint32_t InitBlockInverval = 10000U;
// Amount of seconds the door is locked (doubles every time MaxBlockCounter is reached)
static uint32_t CurrentBlockInverval = InitBlockInverval;

// Block counter message display time
static const uint32_t BlockCounterDisplayTime = 300U;
static uint32_t BlockCounterDisplayStartTick = 0U;
static bool BlockCounterMessageDisplayed = false;

// calibrate for convinant use
static const uint32_t UIDValidationInterval = 100U;
static uint32_t UIDValidationStartTick = 0U;

// Error handling
static ACCESS_CONTROLLER_Fault CurrentFault = ACCESS_CONTROLLER_FAULT_NONE;

 
// Functions

// Getter
ACCESS_CONTROLLER_State ACCESS_CONTROLLER_GetCurrentAccessControllerState(void)
{
    return CurrentAccessControllerState;
}

uint8_t ACCESS_CONTROLLER_GetCurrentBlockCounter(void)
{
    return CurrentBlockCounter;
}

uint32_t ACCESS_CONTROLLER_GetBlockCounterDisplayTime(void)
{
    return BlockCounterDisplayTime;
}

uint32_t ACCESS_CONTROLLER_GetBlockCounterDisplayStartTick(void)
{
    return BlockCounterDisplayStartTick;
}

void ACCESS_CONTROLLER_SetBlockCounterMessageDisplayed(bool Displayed)
{
    BlockCounterMessageDisplayed = Displayed;
}

bool ACCESS_CONTROLLER_GetBlockCounterMessageDisplayed(void)
{
    return BlockCounterMessageDisplayed;
}

/**
//TODO: finish comment
* @brief
* This function updates the state of the access controller
  and handles necessary transition steps:   - updating CurrentAccessControllerState
                                            - SERVO_UnlockDoor()
                                            - SERVO_LockDoor()
                                            - SERVO_StartLockProcessTimer()
                                            - SERVO_StartUnlockedClosedTimer()
*/
static void ChangeState(ACCESS_CONTROLLER_State NewState)
{
    switch (NewState) 
    {
        case LOCKED_READY:
            if(CurrentAccessControllerState == LOCKED_BLOCKED)
            {
                CurrentBlockInverval *= 2;
            }
            CurrentAccessControllerState = LOCKED_READY;
            UIDValidationStartTick = HAL_GetTick();
            break;

        case UNLOCKING:
            
            SERVO_UnlockDoor();
            SERVO_StartLockProcessTimer();
            CurrentAccessControllerState = UNLOCKING;
            break;

        case UNLOCKED_DOOR_CLOSED:
            if(CurrentAccessControllerState == UNLOCKING)
            {
                SERVO_StartUnlockedClosedTimer();
                CurrentAccessControllerState = UNLOCKED_DOOR_CLOSED;
                break;
            }
            else {
                break;
            }
            break;

        case UNLOCKED_DOOR_OPEN:
            switch (CurrentAccessControllerState) 
            {
                case UNINITIALIZED:
                    // TODO: implement alarm sound -> door should be closed and locked on init
                    CurrentAccessControllerState = UNLOCKED_DOOR_OPEN;
                    break;
                case UNLOCKED_DOOR_CLOSED:
                    // maybe here a timer alarm as well 
                    CurrentAccessControllerState = UNLOCKED_DOOR_OPEN;
                    break;
                default: break;
            }
            break;

        case LOCKING:
            if((CurrentAccessControllerState == UNINITIALIZED) ||
            (CurrentAccessControllerState == UNLOCKED_DOOR_OPEN) ||
            (CurrentAccessControllerState == UNLOCKED_DOOR_CLOSED))
            {
                SERVO_LockDoor();
                SERVO_StartLockProcessTimer();
                CurrentAccessControllerState = LOCKING;
                break;
            }
            else
            {
            break;
            }
            break;

        case LOCKED_BLOCKED:
            if(CurrentAccessControllerState == LOCKED_READY)
            {
                CurrentBlockCounter = 0U;
                BlockStartTick = HAL_GetTick();
                CurrentAccessControllerState = LOCKED_BLOCKED;
                break;
            }
            else {
                // UART error could be added
                break;
            }
        
        case FAULT:
            SERVO_StopServo();

            switch (CurrentFault)
            {
                case ACCESS_CONTROLLER_FAULT_NONE:
                    LOG_Write(LOG_ERROR, "CTRL", "Controller fault without specified cause; Servo stopped");
                    break;

                case ACCESS_CONTROLLER_FAULT_LOCK_TIMEOUT:
                    LOG_Write(LOG_ERROR, "CTRL", "Timeout during locking; Servo stopped");
                    break;

                case ACCESS_CONTROLLER_FAULT_UNLOCK_TIMEOUT:
                    LOG_Write(LOG_ERROR, "CTRL", "Timeout during unlocking; Servo stopped");
                    break;

                case ACCESS_CONTROLLER_FAULT_BOLT_SENSORS:
                    LOG_Write(LOG_ERROR, "CTRL", "Contradictory bolt sensor states; Servo stopped");
                    break;

                case ACCESS_CONTROLLER_FAULT_RFID_INIT:
                    LOG_Write(LOG_ERROR, "CTRL", "RFID initialization failed; Servo stopped");
                    break;

                case ACCESS_CONTROLLER_FAULT_STATE:
                    LOG_Write(LOG_ERROR, "CTRL", "Invalid controller state; Servo stopped");
                    break;

                default:
                    LOG_Write(LOG_ERROR, "CTRL", "Unknown fault code; Servo stopped");
                    break;
            }

            CurrentAccessControllerState = FAULT;
            break;

        default:    
            SERVO_StopServo();
            LOG_Write(LOG_ERROR, "CTRL", "Controller fault; Servo stopped");
            CurrentAccessControllerState = FAULT;
            break;
                 

    }
    SSD1306_DisplayCurrentState();
}

// todo error handling
ACCESS_CONTROLLER_Status ACCESS_CONTROLLER_InitRFID(void)
{
//todo: error handling
MFRC522Status Status = MFRC522_ExecuteCommand(SOFT_RESET_COMMAND);

if(Status == MFRC522_OK)
{
    Status = MFRC522_InitTransceive();
}

if(Status != MFRC522_OK)
{
    CurrentFault = ACCESS_CONTROLLER_FAULT_RFID_INIT;
    ChangeState(FAULT);
    return ACCESS_CONTROLLER_ERROR;
}

return ACCESS_CONTROLLER_OK;

}

/*
ACCESS_CONTROLLER_InitState:
- collects state of bolt/door sensors
- sets access controller state accordingly 
*/
ACCESS_CONTROLLER_Status ACCESS_CONTROLLER_InitState(void)
{

SENSORS_UpdateDoorState();
if(SENSORS_UpdateLockState() == false)
{
    CurrentFault = ACCESS_CONTROLLER_FAULT_BOLT_SENSORS;
    ChangeState(FAULT);
    return ACCESS_CONTROLLER_ERROR;
}



if((SENSORS_GetBoltState()== BOLT_LOCKED) && (SENSORS_GetDoorState() == DOOR_CLOSED))
{
    ChangeState(LOCKED_READY);
    return ACCESS_CONTROLLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_UNLOCKED) && (SENSORS_GetDoorState() == DOOR_CLOSED)) {
    
    ChangeState(LOCKING);
    return ACCESS_CONTROLLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_LOCKED) && (SENSORS_GetDoorState() == DOOR_OPEN)) {
    
    ChangeState(UNLOCKING);
    return ACCESS_CONTROLLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_UNLOCKED) && (SENSORS_GetDoorState() == DOOR_OPEN)) {

    ChangeState(UNLOCKED_DOOR_OPEN);
    return ACCESS_CONTROLLER_OK;
}
//TODO: HANDLE BETWEEN AND INVALID!
else if ((SENSORS_GetBoltState()== BOLT_BETWEEN) && (SENSORS_GetDoorState() == DOOR_OPEN)) {
    ChangeState(UNLOCKING);
    return ACCESS_CONTROLLER_OK;
}
else if ((SENSORS_GetBoltState()== BOLT_BETWEEN) && (SENSORS_GetDoorState() == DOOR_CLOSED)) {
    ChangeState(LOCKING);
    return ACCESS_CONTROLLER_OK;
}
else {
return ACCESS_CONTROLLER_INITIALIZING_STATE;
}
}

/*
ACCESS_CONTROLLER_Run:
- collects state of bolt/door sensors and RFID reader
- updates the access controller state accordingly 
*/
ACCESS_CONTROLLER_Status ACCESS_CONTROLLER_Run(void)
{
    if(CurrentAccessControllerState == FAULT)
    {
        return ACCESS_CONTROLLER_ERROR;
    }

    //todo: error handling sensors
    SENSORS_UpdateDoorState();

    if(SENSORS_UpdateLockState() == false)
    {
        CurrentFault = ACCESS_CONTROLLER_FAULT_BOLT_SENSORS;
        ChangeState(FAULT);
        return ACCESS_CONTROLLER_ERROR;
    }

    switch (CurrentAccessControllerState) {

        case LOCKED_READY:
            // Check blockcounter: 
            if(CurrentBlockCounter >= MaxBlockCounter)
            {
                ChangeState(LOCKED_BLOCKED);
                return ACCESS_CONTROLLER_OK;
            }
            if(CurrentBlockCounter > 0U)
            {
                // reader needs to be blocked for 500 ms and displayed, then "ready" again
                SSD1306_DisplayTryBlocked();
            }
            else {
            
            }

            // in case the door gets opened again during the LOCKING state
            if (SENSORS_GetDoorState() == DOOR_OPEN) {
                ChangeState(UNLOCKING);
                return ACCESS_CONTROLLER_OK;
            }

            // UID Validation:
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
                    CurrentBlockCounter = 0U;
                    CurrentBlockInverval = InitBlockInverval;
                    ChangeState(UNLOCKING);
                    return ACCESS_CONTROLLER_OK;
                }
                else if (DetectedUID.UIDLength == 0U) {
                // no RFID detected
                UIDValidationStartTick = HAL_GetTick();
                return ACCESS_CONTROLLER_OK;
                }
                else {
                // invalid UID detected
                ++ CurrentBlockCounter;
                UIDValidationStartTick = HAL_GetTick();
                BlockCounterDisplayStartTick = HAL_GetTick();
                BlockCounterMessageDisplayed = false;
                return ACCESS_CONTROLLER_OK;
                }
            }
            else {
                return ACCESS_CONTROLLER_OK;
            }

        case UNLOCKING:
            if(SENSORS_GetBoltState() == BOLT_UNLOCKED)
            {
                ChangeState(UNLOCKED_DOOR_CLOSED);
                return ACCESS_CONTROLLER_OK;
            }
            else {
                if(SERVO_CheckLockProcessTimerReached())
                {
                    CurrentFault = ACCESS_CONTROLLER_FAULT_UNLOCK_TIMEOUT;
                    ChangeState(FAULT);
                    // todo: better enum
                    return ACCESS_CONTROLLER_ERROR;
                }
                else{
                    return ACCESS_CONTROLLER_OK;
                }
            }

        case UNLOCKED_DOOR_CLOSED:
            if(SENSORS_GetDoorState() == DOOR_OPEN)
            {
                ChangeState(UNLOCKED_DOOR_OPEN);
                return ACCESS_CONTROLLER_OK;
            }
            else if(SERVO_CheckUnlockedClosedTimerReached()) 
            {
                ChangeState(LOCKING);
                return ACCESS_CONTROLLER_OK;
            }
            else {
            return ACCESS_CONTROLLER_OK;
            }

        case UNLOCKED_DOOR_OPEN:

            //when door closed -> locking
            if(SENSORS_GetDoorState() == DOOR_CLOSED)
            {
                ChangeState(LOCKING);
                return ACCESS_CONTROLLER_OK;
            }
            else {
            return ACCESS_CONTROLLER_OK;
            }

        case LOCKING:
                if((SENSORS_GetBoltState() == BOLT_LOCKED) && (SENSORS_GetDoorState() == DOOR_CLOSED))
                {
                    ChangeState(LOCKED_READY);
                    return ACCESS_CONTROLLER_OK;
                }
                else if (SENSORS_GetDoorState() == DOOR_OPEN)
                {
                    // Retract the bolt if the door reopens during locking.
                    ChangeState(UNLOCKING);
                    return ACCESS_CONTROLLER_OK;
                }
                else {
                    if(SERVO_CheckLockProcessTimerReached() == true)
                    {
                        CurrentFault = ACCESS_CONTROLLER_FAULT_LOCK_TIMEOUT;
                        ChangeState(FAULT);
                        return ACCESS_CONTROLLER_ERROR;
                    }
                    else{
                        return ACCESS_CONTROLLER_OK;
                    }
                }
        
        case LOCKED_BLOCKED:
            // check if block time is reached  
            if((HAL_GetTick() - BlockStartTick) >= CurrentBlockInverval)
            {
                ChangeState(LOCKED_READY);
                return ACCESS_CONTROLLER_OK;
            }
            else {
                return ACCESS_CONTROLLER_OK;
            }

        case UNINITIALIZED:
            return ACCESS_CONTROLLER_ERROR;
        
        case FAULT:
            return ACCESS_CONTROLLER_ERROR;

        default:
            CurrentFault = ACCESS_CONTROLLER_FAULT_STATE;
            ChangeState(FAULT);
            return ACCESS_CONTROLLER_ERROR;
    
    }
}
