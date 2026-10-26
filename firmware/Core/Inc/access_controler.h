#ifndef ACCESS_CONTROLER_H
#define ACCESS_CONTROLER_H

/*
TODO: 
- state maschine for access controler 
*/

typedef enum
{
    LOCKED_READY = 0x00, // door is locked and can be unlocked with whitelisted UID
    UNLOCKING = 0x01, // bolt unlocks
    LOCKING = 0x02, //bolt locks
    LOCKED_BLOCKED = 0x03, // door cant be unlocked
    UNLOCKED_DOOR_CLOSED = 0x04, // bolt unlocked but door is closed
    UNLOCKED_DOOR_OPEN = 0x05, // bolt unlocked and door is open
    UNINITIALIZED = 0x06
    
} ACCESS_CONTROLER_State;

typedef enum
{
    ACCESS_CONTROLER_OK = 0x00,
    ACCESS_CONTROLER_ERROR = 0x01,
    ACCESS_CONTROLER_INIT_FAIL = 0x02,
    ACCESS_CONTROLER_STATE_ERROR = 0x03,
    ACCESS_CONTROLER_SENSOR_ERROR = 0x04,
    ACCESS_CONTROLER_INITIALIZING_STATE = 0x05

} ACCESS_CONTROLER_Status;

ACCESS_CONTROLER_Status ACCESS_CONTROLER_InitRFID(void);

ACCESS_CONTROLER_Status ACCESS_CONTROLER_InitState(void);

ACCESS_CONTROLER_Status ACCESS_CONTROLER_Run(void);

ACCESS_CONTROLER_State ACCESS_CONTROLER_GetCurrentAccessControlerState(void);






























#endif