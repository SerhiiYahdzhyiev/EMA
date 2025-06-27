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
        printf("Failed to finalize EMA: %d\n", err);
        return 1;
    }

    PluginPtrArray plugins = EMA_get_plugins();
    printf("Number of plugins: %lu\n", plugins.size);
    for(size_t i = 0; i < plugins.size; ++i)
        printf("Plugin %lu: %s\n", i, EMA_get_plugin_name(plugins.array[i]));

    DevicePtrArray devices = EMA_get_devices();
    printf("Number of devices: %lu\n", devices.size);
    for(size_t i = 0; i < devices.size; ++i)
        printf("Device %lu: %s\n", i, EMA_get_device_name(devices.array[i]));

    /* Lower-level API. */
    static thread_local Region *region = NULL;
    EMA_region_define(&region, "r1", NULL, "", 0, "");

    EMA_region_begin(region);

    sleep(2);

    EMA_region_end(region);

    printf("Output: \n");
    EMA_print_all(stdout);

    printf("Finalizing EMA...\n");
    err = EMA_finalize();
    if (err)
    {
        printf("Failed to finalize EMA: %d\n", err);
        return 1;
    }

    return 0;
}
