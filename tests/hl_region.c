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
    for(size_t i = 0; i < devices.size; ++i)
        printf("Device %lu: %s\n", i, EMA_get_device_name(devices.array[i]));

    EMA_REGION_DECLARE(region);
    EMA_REGION_DEFINE(&region, "r2");

    EMA_REGION_BEGIN(region);

    sleep(2);

    EMA_REGION_END(region);

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
