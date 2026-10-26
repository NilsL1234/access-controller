# include "access_controler.h"
# include "rfid.h"
# include "sensors.h"
# include "servo.h"
# include "main.h"
# include <stdint.h>


// Variables

static ACCESS_CONTROLER_State CurrentAccessControlerState = UNINITIALIZED;
static uint8_t BlockCounter = 0U;
static const uint8_t MaxBlockCounter = 3U;
static uint32_t BlockTimerSec = 0U;
// amount of seconds the door is locked (doubles every time MaxBlockCounter is reached)
static uint32_t CurrentBlockTimeMS = 10000U;

 
//Functions

ACCESS_CONTROLER_State ACCESS_CONTROLER_GetCurrentAccessControlerState(void)
{
    return CurrentAccessControlerState;
}

static void ChangeState(ACCESS_CONTROLER_State NewState)
{
    switch (NewState) {
        case LOCKED_READY:

        switch (CurrentAccessControlerState) 
        {
            case UNINITIALIZED:
                CurrentAccessControlerState = LOCKED_READY;
                return;
            case LOCKING:
                CurrentAccessControlerState = LOCKED_READY;
                return;
            case LOCKED_BLOCKED:
                CurrentBlockTimeMS *= 2;
                CurrentAccessControlerState = LOCKED_READY;
                return;
            default: return;
        }

        case UNLOCKING:

        switch (CurrentAccessControlerState) 
        {
            case UNINITIALIZED:
                SERVO_UnlockDoor();
                SERVO_StartLockProcessTimer();
                CurrentAccessControlerState = UNLOCKING;
                return;
            case LOCKED_READY:
                BlockCounter = 0U;
                SERVO_UnlockDoor();
                SERVO_StartLockProcessTimer();
                CurrentAccessControlerState = UNLOCKING;
                return;
            default: return;
        }

        case UNLOCKED_DOOR_CLOSED:

        if(CurrentAccessControlerState == UNLOCKING)
        {
            SERVO_StartUnlockedClosedTimer();
            CurrentAccessControlerState = UNLOCKED_DOOR_CLOSED;
            return;
        }
        else {
            return;
        }

        case UNLOCKED_DOOR_OPEN:

        switch (CurrentAccessControlerState) 
        {
            case UNINITIALIZED:
                // TODO: implement alarm sound -> door should be closed and locked on init
                CurrentAccessControlerState = UNLOCKED_DOOR_OPEN;
                return;
            case UNLOCKED_DOOR_CLOSED:
                // maybe here a timer alarm as well 
                CurrentAccessControlerState = UNLOCKED_DOOR_OPEN;
            default: return;
        }

        case LOCKING:

        if((CurrentAccessControlerState == UNINITIALIZED) ||
           (CurrentAccessControlerState == UNLOCKED_DOOR_OPEN) ||
           (CurrentAccessControlerState == UNLOCKED_DOOR_CLOSED))
        {
            SERVO_LockDoor();
            SERVO_StartLockProcessTimer();
            CurrentAccessControlerState = LOCKING;
            return;
        }
        else
        {
           return;
        }

        case LOCKED_BLOCKED:

        if(CurrentAccessControlerState == LOCKED_READY)
        {
            BlockCounter = 0U;
            BlockTimerSec = HAL_GetTick();
            CurrentAccessControlerState = LOCKED_BLOCKED;
            return;
        }
        else {
            // UART error could be added
            return;
        }

        default: return;

    }
}





// todo error handling
ACCESS_CONTROLER_Status ACCESS_CONTROLER_InitRFID(void)
{
//todo: error handling
MFRC522Status Status = MFRC522_ExecuteCommand(SOFT_RESET_COMMAND);
Status = MFRC522_InitTransceive();

return ACCESS_CONTROLER_OK;

}

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

ACCESS_CONTROLER_Status ACCESS_CONTROLER_Run(void)
{

    //todo: error handling sensors
    SENSORS_UpdateDoorState();
    SENSORS_UpdateLockState();

    switch (CurrentAccessControlerState) {
        case LOCKED_READY:

        // check blockcounter 
        if(BlockCounter >= MaxBlockCounter)
        {
            ChangeState(LOCKED_BLOCKED);
            return ACCESS_CONTROLER_OK;
        }

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
        return ACCESS_CONTROLER_OK;
        }
        else {
        // invalid UID detected
        ++ BlockCounter;
        return ACCESS_CONTROLER_OK;
        }

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
        if((HAL_GetTick() - BlockTimerSec) >= CurrentBlockTimeMS)
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