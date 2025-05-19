#include "device.h"

const char *EMA_device_get_name(const Device* device)
{
    return device->name;
}

const char *EMA_device_get_type(const Device* device)
{
    return device->type;
}
