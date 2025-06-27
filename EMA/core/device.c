#include "device.h"

const char *EMA_device_get_name(const Device* device)
{
    return device->name;
}

const char *EMA_get_device_type(const Device* device)
{
    return device->type;
const char *EMA_get_device_uid(const Device* device)
{
    return device->uid;
}
