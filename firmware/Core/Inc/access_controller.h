#ifndef ACCESS_CONTROLLER_H
#define ACCESS_CONTROLLER_H

#include <stdint.h>

// Enums
typedef enum
{
    LOCKED_READY = 0x00, // door is locked and can be unlocked with whitelisted UID
    UNLOCKING = 0x01, // bolt unlocks
    LOCKING = 0x02, //bolt locks
    LOCKED_BLOCKED = 0x03, // door cant be unlocked
    UNLOCKED_DOOR_CLOSED = 0x04, // bolt unlocked but door is closed
    UNLOCKED_DOOR_OPEN = 0x05, // bolt unlocked and door is open
    UNINITIALIZED = 0x06,
    FAULT = 0x07
    
} ACCESS_CONTROLLER_State;

typedef enum
{
    ACCESS_CONTROLLER_OK = 0x00,
    ACCESS_CONTROLLER_ERROR = 0x01,
    ACCESS_CONTROLLER_INITIALIZING_STATE = 0x02

} ACCESS_CONTROLLER_Status;

typedef enum
{
    ACCESS_CONTROLLER_FAULT_NONE = 0x00,
    ACCESS_CONTROLLER_FAULT_LOCK_TIMEOUT = 0x01,
    ACCESS_CONTROLLER_FAULT_UNLOCK_TIMEOUT = 0x02,
    ACCESS_CONTROLLER_FAULT_BOLT_SENSORS = 0x03,
    ACCESS_CONTROLLER_FAULT_RFID_INIT = 0x04,
    ACCESS_CONTROLLER_FAULT_SERVO = 0x05,
    ACCESS_CONTROLLER_FAULT_STATE = 0x06,
    ACCESS_CONTROLLER_FAULT_RFID_COMMUNICATION = 0x07,
    ACCESS_CONTROLLER_FAULT_RFID_VALIDATION = 0x08
} ACCESS_CONTROLLER_Fault;

// Getter
ACCESS_CONTROLLER_State ACCESS_CONTROLLER_GetCurrentAccessControllerState(void);

uint8_t ACCESS_CONTROLLER_GetCurrentBlockCounter(void);

uint32_t ACCESS_CONTROLLER_GetBlockCounterDisplayTime(void);

uint32_t ACCESS_CONTROLLER_GetBlockCounterDisplayStartTick(void);


// Functions
ACCESS_CONTROLLER_Status ACCESS_CONTROLLER_InitRFID(void);

ACCESS_CONTROLLER_Status ACCESS_CONTROLLER_InitState(void);

ACCESS_CONTROLLER_Status ACCESS_CONTROLLER_Run(void);

#endif
