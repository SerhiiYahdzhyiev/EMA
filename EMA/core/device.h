#ifndef EMA_CORE_DEVICE_H
#define EMA_CORE_DEVICE_H

#include "device.user.h"
#include "overflow.h"

typedef struct Plugin Plugin;

typedef struct Device
{
    Plugin* plugin;
    const char *name;
    void *data;
    OverflowData overflow;
} Device;

#endif
