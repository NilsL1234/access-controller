# include "access_controller.h"
# include "rfid.h"
# include "sensors.h"
# include "servo.h"
# include "main.h"
# include "oled.h"
# include "log.h"
#include <stddef.h>
# include <stdint.h>
# include <stdbool.h>
#include <stdio.h>

// Variables

static ACCESS_CONTROLLER_State CurrentAccessControllerState = UNINITIALIZED;
static uint8_t CurrentBlockCounter = 0U;
static const uint8_t MaxBlockCounter = 3U;
static uint32_t BlockStartTick = 0U;
static const uint32_t InitBlockInverval = 10000U;
// Amount of ms the door is locked (doubles every time MaxBlockCounter is reached)
static uint32_t CurrentBlockInverval = InitBlockInverval;
// Block interval caps at 5min
static const uint32_t MaxBlockInterval = 300000U;

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

static const char *GetStateString(ACCESS_CONTROLLER_State State)
{
    switch (State) {
        case LOCKED_READY:
            return "LOCKED_READY";
        case UNLOCKING:
            return "UNLOCKING";
        case LOCKING:
            return "LOCKING";
        case LOCKED_BLOCKED:
            return "LOCKED_BLOCKED";
        case UNLOCKED_DOOR_CLOSED:
            return "UNLOCKED_DOOR_CLOSED";
        case UNLOCKED_DOOR_OPEN:
            return "UNLOCKED_DOOR_OPEN";
        case UNINITIALIZED:
            return "UNINITIALIZED";
        case FAULT:
            return "FAULT";
        default:
            return "UNKNOWN";
    }
}


static bool GetUIDString(const uint8_t *UID, size_t UIDSize, char* String, size_t StringSize)
{
    if((UID == NULL) || (UIDSize == 0) || (UIDSize > MAX_UID_LENGTH) || (String == NULL) ||  StringSize < UIDSize * 3 + 1)
    {
        return false;
    }

    size_t Pos = 0;

    for(size_t i=0; i<UIDSize; ++i)
    {
        Pos += snprintf(&String[Pos], StringSize - Pos, "%02X ", UID[i]);
    }

    return true;
}

static void HandleFault(ACCESS_CONTROLLER_Fault Fault)
{
    SERVO_StopServo();
    CurrentFault = Fault;
    CurrentAccessControllerState = FAULT;

    switch (Fault)
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

        case ACCESS_CONTROLLER_FAULT_SERVO:
            LOG_Write(LOG_ERROR, "CTRL", "Invalid sensor state during SERVO_LockDoor; Servo stopped");
            break;

        case ACCESS_CONTROLLER_FAULT_RFID_COMMUNICATION:
            LOG_Write(LOG_ERROR, "CTRL", "Error in RFID communication; Servo stopped");
            break;

        default:
            LOG_Write(LOG_ERROR, "CTRL", "Unknown fault code; Servo stopped");
            break;
    }
    SSD1306_DisplayCurrentState();
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

    ACCESS_CONTROLLER_State PreviousState = CurrentAccessControllerState;

    switch (NewState) 
    {
        case LOCKED_READY:
            if(CurrentAccessControllerState == LOCKED_BLOCKED)
            {
            CurrentBlockInverval *= 2;
            if(CurrentBlockInverval > MaxBlockInterval)
                {
                    CurrentBlockInverval = MaxBlockInterval;
                }
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
                if(SERVO_LockDoor() != SERVO_OK)
                {
                    HandleFault(ACCESS_CONTROLLER_FAULT_SERVO);
                    break;
                }

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

                char Message[64];
                snprintf(Message, sizeof(Message), "Blocked for %lu ms", CurrentBlockInverval);
                LOG_Write(LOG_INFO, "CTRL", Message);
                break;
            }
            else {
                // UART error could be added
                break;
            }
        
        // case FAULT: is entered only via HandleFault()

        default:    
            HandleFault(ACCESS_CONTROLLER_FAULT_STATE);
            break;
    }

    if(CurrentAccessControllerState != PreviousState)
    {
        char Message[64];
        snprintf(Message, sizeof(Message), "New state %s", GetStateString(CurrentAccessControllerState));
        LOG_Write(LOG_INFO, "CTRL", Message);
    }

    SSD1306_DisplayCurrentState();
}

ACCESS_CONTROLLER_Status ACCESS_CONTROLLER_InitRFID(void)
{

MFRC522Status Status = MFRC522_ExecuteCommand(SOFT_RESET_COMMAND);

if(Status == MFRC522_OK)
{
    Status = MFRC522_InitTransceive();
}

if(Status != MFRC522_OK)
{
    HandleFault(ACCESS_CONTROLLER_FAULT_RFID_INIT);
    return ACCESS_CONTROLLER_ERROR;
}

LOG_Write(LOG_INFO, "RFID", "Init successful");
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
    HandleFault(ACCESS_CONTROLLER_FAULT_BOLT_SENSORS);
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
        HandleFault(ACCESS_CONTROLLER_FAULT_BOLT_SENSORS);
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
                

                switch (MFRC522StatusDetection)
                {
                    case MFRC522_UID_DETECTED:
                        {
                        bool UIDIsValid = false;
                        (void)MFRC522_ValidateUID(&DetectedUID, DetectedUID.UIDLength, &UIDIsValid);
                        if(UIDIsValid)
                        {
                            char UIDString [MAX_UID_LENGTH * 3 + 1];
                            if(GetUIDString(DetectedUID.UID, DetectedUID.UIDLength, UIDString, sizeof(UIDString)))
                            {
                                char Message[64];
                                snprintf(Message, sizeof(Message), "Valid UID detected %s", UIDString);
                                LOG_Write(LOG_INFO, "RFID", Message);
                            }
                            else {
                                LOG_Write(LOG_WARN, "CTRL", "Logging valid UID failed");
                            }

                            CurrentBlockCounter = 0U;
                            CurrentBlockInverval = InitBlockInverval;
                            ChangeState(UNLOCKING);
                            return ACCESS_CONTROLLER_OK;
                        }
                        else {
                        // invalid UID detected

                        // Logging invalid UID
                        char UIDString [MAX_UID_LENGTH * 3 + 1];
                        if(GetUIDString(DetectedUID.UID, DetectedUID.UIDLength, UIDString, sizeof(UIDString)))
                        {
                            char Message[64];
                            snprintf(Message, sizeof(Message), "Invalid UID detected %s", UIDString);
                            LOG_Write(LOG_INFO, "RFID", Message);
                        }
                        else {
                            LOG_Write(LOG_WARN, "CTRL", "Logging invalid UID failed");
                        }

                        ++ CurrentBlockCounter;
                        char BlockCounterLog[64];
                        snprintf(BlockCounterLog, sizeof(BlockCounterLog), "Current number of blocks %u/%u", CurrentBlockCounter, MaxBlockCounter);
                        LOG_Write(LOG_INFO, "RFID", BlockCounterLog);

                        UIDValidationStartTick = HAL_GetTick();
                        BlockCounterDisplayStartTick = HAL_GetTick();
                        BlockCounterMessageDisplayed = false;
                        return ACCESS_CONTROLLER_OK;
                        }
                        break;
                        }
                    case MFRC522_NO_UID_DETECTED:
                        UIDValidationStartTick = HAL_GetTick();
                        return ACCESS_CONTROLLER_OK;
                        
                    case MFRC522_OK:
                        return ACCESS_CONTROLLER_OK;
                    default:
                        HandleFault(ACCESS_CONTROLLER_FAULT_RFID_COMMUNICATION);
                        return ACCESS_CONTROLLER_ERROR;
                }

                // todo: MFRC522StatusValidation errorhandling
            }
            return ACCESS_CONTROLLER_OK;

        case UNLOCKING:
            if(SENSORS_GetBoltState() == BOLT_UNLOCKED)
            {
                ChangeState(UNLOCKED_DOOR_CLOSED);
                return ACCESS_CONTROLLER_OK;
            }
            else {
                if(SERVO_CheckLockProcessTimerReached())
                {
                    HandleFault(ACCESS_CONTROLLER_FAULT_UNLOCK_TIMEOUT);
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
                        HandleFault(ACCESS_CONTROLLER_FAULT_LOCK_TIMEOUT);
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
            HandleFault(ACCESS_CONTROLLER_FAULT_STATE);
            return ACCESS_CONTROLLER_ERROR;
    
    }
}
