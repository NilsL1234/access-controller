#ifndef SERVO_H
#define SERVO_H

# include <stdbool.h>

typedef enum
{
    SERVO_OK = 0x00,
    SERVO_ERROR = 0x01
} Servo_Status;

typedef enum
{
    SERVO_DOOR_OPEN = 0x00,
    SERVO_DOOR_CLOSED = 0x01
} Servo_Door_State;

void SERVO_StartLockProcessTimer(void);

void SERVO_StartUnlockedClosedTimer(void);

bool SERVO_CheckLockProcessTimerReached(void);

bool SERVO_CheckUnlockedClosedTimerReached(void);

Servo_Status SERVO_UnlockDoor(void);

Servo_Status SERVO_LockDoor(void);

void SERVO_StopServo(void);







#endif