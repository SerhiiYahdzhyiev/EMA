#include <stdlib.h>
#include <string.h>

#include <EMA/core/device.h>
#include <EMA/core/registry.h>
#include <EMA/user.h>
#include <EMA/utils/time.h>
#include "filter.h"
#include "region.h"
#include "region_store.h"

#define ASSERT(COND, RET) \
    if( !(COND) ) \
        return RET;

#define ASSERT_1(COND) ASSERT(COND, 1)

extern PluginRegistry registry;

/* Public interface. */
int EMA_region_create_and_init(
    Region **region,
    const char* idf,
    Filter *filter,
    const char* file,
    unsigned int line,
    const char* func
) {
    DevicePtrArray devices = registry.devices;
    if( filter )
        devices = filter->apply(registry.devices, filter);

    *region = (Region*) malloc(sizeof(Region));

    (*region)->measurements.array = (Measurement*) malloc(
        sizeof(Measurement) * devices.size);
    (*region)->measurements.size = devices.size;

    for(size_t i = 0; i < (*region)->measurements.size; ++i)
    {
        Measurement* measurement = (*region)->measurements.array + i;
        measurement->device = devices.array[i];
        measurement->energy_start = 0;
        measurement->energy_result = 0;
        measurement->time_start = 0;
        measurement->time_result = 0;
    }

    (*region)->visits = 0;
    (*region)->idf = strdup(idf);
    (*region)->file = strdup(file);
    (*region)->function = strdup(func);
    (*region)->line = line;

    if( filter )
        free(devices.array);

    return 0;
}

int EMA_region_begin(Region *region)
{
    ++region->visits;

    for(size_t i = 0; i < region->measurements.size; ++i)
    {
        Measurement* measurement = region->measurements.array + i;
        measurement->time_start = EMA_get_time_in_us();
        measurement->energy_start = EMA_plugin_get_energy_uj(
            measurement->device);
    }
    return 0;
}

int EMA_region_end(Region *region)
{
    for(size_t i = 0; i < region->measurements.size; ++i)
    {
        Measurement* measurement = region->measurements.array + i;
        measurement->time_result +=
            EMA_get_time_in_us() - measurement->time_start;
        measurement->energy_result += EMA_plugin_get_energy_uj(
            measurement->device) - measurement->energy_start;
    }
    return 0;
}

int EMA_region_finalize(Region *region)
{
    free(region->idf);
    free(region->file);
    free(region->function);
    free(region->measurements.array);
    free(region);
    return 0;
}

/* High-level API. */
int EMA_region_define(
    Region **region,
    const char* idf,
    Filter *filter,
    const char* file,
    unsigned int line,
    const char* func
) {
    int err;

    if( *region )
        return 0;

    err = EMA_thread_init();
    ASSERT_1(err == 0);

    err = EMA_region_create_and_init(region, idf, filter, file, line, func);
    ASSERT_1(err == 0);

    RegionStore* store = EMA_thread_get_region_store();
    ASSERT_1(store);

    EMA_region_store_set(store, *region);

    return 0;
}
