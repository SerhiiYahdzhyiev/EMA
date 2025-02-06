#ifndef EMA_CORE_DEVICE_USER_H
#define EMA_CORE_DEVICE_USER_H

#include <stddef.h>

typedef struct Device Device;

/**
 * This struct defines an array of `Device` objects.
 *
 * Members:
 *   array: Array of `Device` objects.
 *   size: Number of elements in `array`.
 */
typedef struct
{
    Device* array;
    size_t size;
} DeviceArray;

/**
 * This struct defines an array of `Device` pointers.
 *
 * Members:
 *   array: Array of `Device` pointers.
 *   size: Number of elements in `array`.
 */
typedef struct
{
    Device** array;
    size_t size;
} DevicePtrArray;

/**
 * This function reads the name of a given device.
 *
 * @param device: `Device` from which the name is to be read.
 *
 * @returns The name of the given device.
 */
const char *EMA_device_get_name(const Device* device);

/**
 * This function reads the unique identifier of a given device.
 *
 * @param device: `Device` from which the uid is to be read.
 *
 * @returns The uid of the given device.
 */
const char *EMA_device_get_uid(const Device* device);

#endif
