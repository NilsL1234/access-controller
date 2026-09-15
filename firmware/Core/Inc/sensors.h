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

bool UpdateLockState(void);

BoltState GetBoltState(void);

#endif