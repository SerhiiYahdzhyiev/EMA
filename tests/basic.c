#include <stddef.h>
#include <stdio.h>
#include <threads.h>

#include <unistd.h>

#include <EMA.h>

int main(int argc, char **argv)
{
    printf("Initializing EMA...\n");
    int err = EMA_init(NULL);
    if( err )
    {
        printf("Failed to initialize EMA: %d\n", err);
        return 1;
    }

    PluginPtrArray plugins = EMA_get_plugins();
    printf("Number of plugins: %lu\n", plugins.size);
    for(size_t i = 0; i < plugins.size; ++i)
        printf("Plugin %lu: %s\n", i, EMA_get_plugin_name(plugins.array[i]));

    DevicePtrArray devices = EMA_get_devices();
    printf("Number of devices: %lu\n", devices.size);
    for(size_t i = 0; i < devices.size; ++i) {
        printf("Device %lu: %s\n", i, EMA_get_device_name(devices.array[i]));
        printf("Device %lu: uid: %s\n", i, EMA_get_device_uid(devices.array[i]));
    }

    /* Filter. */
    Filter *filter = EMA_filter_exclude_plugin("NVML");

    /* Lower-level API. */
    printf("Region 1\n");
    static thread_local Region *region = NULL;
    EMA_region_define(&region, "r1", filter, "", 0, "");

    EMA_region_begin(region);

    sleep(2);

    EMA_region_end(region);

    /* Higher-level API. */
    printf("Region 2\n");
    EMA_REGION_DECLARE(region2);
    EMA_REGION_DEFINE_WITH_FILTER(&region2, "r2", filter);

    EMA_REGION_BEGIN(region2);

    sleep(2);

    EMA_REGION_END(region2);
   
    EMA_filter_finalize(filter);

    printf("Output:\n");
    EMA_print_all(stdout);

    printf("Finalizing EMA...\n");
    err = EMA_finalize();
    if (err) {
        printf("Failed to finalize EMA: %d\n", err);
        return 1;
    }

    return 0;
}
