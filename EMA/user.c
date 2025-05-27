#include <stdlib.h>

#include <unistd.h>

#include <EMA/core/registry.h>
#ifdef EMA_HAVE_NVML
    #include <EMA/plugins/plugin_nvml.h>
#endif
#include <EMA/plugins/plugin_rapl.h>
#include <EMA/region/output.h>
#include <EMA/region/region_store.h>

#include "user.h"

extern PluginRegistry registry;

/* Output. */
int EMA_print_results()
{
    int ret;
    char* filename;

    ret = asprintf(&filename, "output.EMA.%u", getpid());
    if( ret == -1 )
        return 1;

    FILE *f = fopen(filename, "w");
    free(filename);
    if( !f )
        return 1;

    ret = EMA_print_all(f);
    fclose(f);
    if( ret != 0 )
        return ret;

    return 0;
}


/* Initialization and cleanup. */
int EMA_init(EMA_init_cb callback)
{
    int err;

    #ifdef EMA_HAVE_NVML
    err = register_nvml_plugin();
    if( err )
        return err;
    #endif

    err = register_rapl_plugin();
    if( err )
        return err;

    if( callback )
    {
        int err = callback();
        if( err )
            return err;
    }

    registry.devices.size = 0;
    for(size_t i = 0; i < registry.plugins.size; ++i)
    {
        Plugin *plugin = registry.plugins.array[i];
        int err = EMA_plugin_init(plugin);
        if( err )
            continue;

        DeviceArray devices = plugin->cbs.get_devices(plugin);
        registry.devices.size += devices.size;
    }

    registry.devices.array = malloc(registry.devices.size * sizeof(Device*));
    size_t k = 0;
    for(size_t i = 0; i < registry.plugins.size; ++i)
    {
        Plugin *plugin = registry.plugins.array[i];
        DeviceArray devices = plugin->cbs.get_devices(plugin);
        for(size_t j = 0; j < devices.size; ++j)
            registry.devices.array[k++] = &devices.array[j];
    }

    err = start_overflow_tracking(&registry.devices);
    if( err )
        return err;

    return 0;
}

int EMA_finalize()
{
    int ret = EMA_print_results();
    if( ret != 0 )
        return ret;

    stop_overflow_tracking(&registry.devices);

    for(int i = 0; i < registry.plugins.size; ++i)
    {
        Plugin *plugin = registry.plugins.array[i];
        plugin->cbs.finalize(plugin);
        free(plugin);
    }

    if( registry.plugins.array )
        free(registry.plugins.array);

    if( registry.devices.array )
        free(registry.devices.array);

    return EMA_region_stores_finalize();
}


/* Plugin interface. */
int EMA_plugin_init(Plugin* plugin)
{
    return plugin->cbs.init(plugin);
}

DevicePtrArray EMA_get_plugin_devices(const Plugin* plugin)
{
    DeviceArray devices = plugin->cbs.get_devices(plugin);
    DevicePtrArray device_ptrs;
    device_ptrs.size = devices.size;
    device_ptrs.array = malloc(device_ptrs.size * sizeof(Device*));
    for(int i = 0; i < device_ptrs.size; ++i)
        device_ptrs.array[i] = devices.array + i;
    return device_ptrs;
}

unsigned long long EMA_get_energy_uj(const Device* device)
{
    return device->plugin->cbs.get_energy_uj(device);
}

unsigned long long EMA_plugin_get_energy_uj(const Device* device)
{
    return EMA_get_handled_energy_uj(device);
}

int EMA_plugin_finalize(Plugin* plugin)
{
    return plugin->cbs.finalize(plugin);
}


/* Plugin interface. */
const char* EMA_get_plugin_name(const Plugin* plugin)
{
    return plugin->name;
}


/* Device interface. */
const char* EMA_get_device_name(const Device* device)
{
    return device->name;
}

/* Global interface. */
PluginPtrArray EMA_get_plugins()
{
    return registry.plugins;
}

DevicePtrArray EMA_get_devices()
{
    return registry.devices;
}
