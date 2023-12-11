#ifndef EMA_REGION_FILTER_USER_H
#define EMA_REGION_FILTER_USER_H

#include "../core/device.user.h"

typedef struct Filter Filter;

/**
 * @typedef EMA_device_filter_cb
 *
 * Callback type for `Filter` objects.
 *
 * @param devices: Array of `Device` objects that are to be filtered.
 * @param filter: The `Filter` object this callback can operate on.
 */
typedef DevicePtrArray (*EMA_device_filter_cb)(
    DevicePtrArray devices, Filter *filter);

/**
 * This struct defines a `Filter` used for filtering `Device` objects.
 *
 * Members:
 *   apply: A callback that defines a filter function on devices.
 *   data: A void pointer to store arbitrary data for use in the callback.
 */
typedef struct Filter
{
    EMA_device_filter_cb apply;
    void *data;
} Filter;

/**
 * This function creates a `Filter`.
 *
 * @param cb: Callback function that defines the filter logic.
 * @param data: Void pointer used to pass data to the callback function.
 *
 * @returns The filter of type `Filter`.
 */
Filter *EMA_filter_create(EMA_device_filter_cb cb, void *data);

/**
 * This function finalizes the `Filter` object and clears its memory.
 *
 * @param filter: Pointer to a `Filter` that is to be finalized.
 */
void EMA_filter_finalize(Filter *filter);

/**
 * This function provides a predefined `Filter` to exclude a `Plugin`.
 *
 * @param plugin_name: The name of the `Plugin` to exclude.
 *
 * @returns A `Filter` that excludes the `Plugin` named `plugin_name`.
 */
Filter *EMA_filter_exclude_plugin(const char* plugin_name);

#endif
