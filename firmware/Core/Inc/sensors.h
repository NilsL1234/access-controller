#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    BOLT_INIT,
    BOLT_LOCKED,
    BOLT_UNLOCKED,
    BOLT_BETWEEN,
    BOLT_INVALID,
    BOLT_ERROR
} BoltState;

typedef enum
{
    BOLT_SENSOR_INIT,
    BOLT_SENSOR_ACTIVE,
    BOLT_SENSOR_INACTIVE
} BoltSensorState;

typedef enum
{
    DOOR_INIT,
    DOOR_CLOSED,
    DOOR_OPEN,
    DOOR_INVALID
} DoorState;

typedef enum
{
    DOOR_SENSOR_INIT,
    DOOR_SENSOR_ACTIVE,
    DOOR_SENSOR_INACTIVE

} DoorSensorState;

BoltState SENSORS_GetBoltState(void);

DoorState SENSORS_GetDoorState(void);

bool SENSORS_UpdateLockState(void);

bool SENSORS_UpdateDoorState(void);

#endif