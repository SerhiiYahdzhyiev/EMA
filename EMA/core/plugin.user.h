#ifndef EMA_CORE_PLUGIN_USER_H
#define EMA_CORE_PLUGIN_USER_H

#include <stddef.h>

typedef struct Plugin Plugin;

/**
 * This struct defines an array of `Plugin` objects.
 *
 * Members:
 *   array: Array of `Plugin` objects.
 *   size: Number of elements in `array`.
 */
typedef struct
{
    Plugin* array;
    size_t size;
} PluginArray;

/**
 * This struct defines an array of `Plugin` pointers.
 *
 * Members:
 *   array: Array of `Plugin` pointers.
 *   size: Number of elements in `array`.
 */
typedef struct
{
    Plugin** array;
    size_t size;
} PluginPtrArray;

#endif
