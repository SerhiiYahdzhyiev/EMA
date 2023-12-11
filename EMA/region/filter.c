#include <stdlib.h>
#include <string.h>

#include <EMA/core/device.h>
#include <EMA/core/plugin.h>
#include "filter.h"

Filter *EMA_filter_create(EMA_device_filter_cb cb, void *data)
{
    Filter *filter = malloc(sizeof(Filter));
    filter->apply = cb;
    filter->data = data;
    return filter;
}

void EMA_filter_finalize(Filter *filter)
{
    free(filter->data);
    free(filter);
}

/* Plugin filters. */
static
DevicePtrArray _filter_exclude_plugin(
    DevicePtrArray devices, Filter* filter)
{
    const char *plugin_name = filter->data;

    DevicePtrArray filtered_devices = { .array = NULL, .size = 0 };
    for(size_t i = 0; i < devices.size; ++i)
    {
        Device *device = devices.array[i];
        if( strcmp(device->plugin->name, plugin_name) == 0)
            continue;
        ++filtered_devices.size;
    }

    filtered_devices.array = malloc(sizeof(Device*) * filtered_devices.size);

    size_t j = 0;
    for(size_t i = 0; i < devices.size; ++i)
    {
        Device *device = devices.array[i];
        if( strcmp(device->plugin->name, plugin_name) == 0)
            continue;
        filtered_devices.array[j++] = device;
    }

    return filtered_devices;
}

Filter *EMA_filter_exclude_plugin(const char* plugin_name)
{
    return EMA_filter_create(_filter_exclude_plugin, strdup(plugin_name));
}
