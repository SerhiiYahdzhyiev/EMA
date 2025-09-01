#include <stdlib.h>
#include <unistd.h>

#include <EMA/core/device.h>
#include <EMA/core/overflow.h>
#include <EMA/core/plugin.h>
#include <EMA/core/registry.h>
#include <EMA/core/utils.h>
#include <EMA/utils/error.h>

#include <EMA/ext/libsmu/libsmu.h>

smu_obj_t obj;

/* Typedefs */

/* Plugin interface */
static
int ryzen_plugin_init(Plugin* plugin)
{
    unsigned int result;

    // Userspace library requires root permissions to access driver.
    if (getuid() != 0 && geteuid() != 0) {
        fprintf(stderr, "Program must be run as root.\n");
        return 1;
    }

    // Initialize the library for use with the program.
    if (smu_init(&obj) != SMU_Return_OK) {
        fprintf(stderr, "Error initializing userspace library.\n");
        return 1;
    }
    return 0;
}

static
DeviceArray ryzen_plugin_get_devices(const Plugin* plugin)
{
    DeviceArray devices = {
        .size = 0,
        .array = NULL
    };
    return devices;
}

static
unsigned long long ryzen_get_energy_update_interval(const Device* device)
{
    return 100;
}

static
unsigned long long ryzen_get_energy_max(const Device* device)
{
    return 1e6;
}

static
unsigned long long ryzen_plugin_get_energy_uj(const Device* device)
{
    return 0;
}

static
int ryzen_plugin_finalize(Plugin* plugin)
{
    // Cleanup after library use has ended.
    smu_free(&obj);

    return 0;
}

/* Extern */
Plugin* create_ryzen_plugin(const char* name)
{
    Plugin* plugin = malloc(sizeof(Plugin));
    ASSERT_OR_NULL(plugin);

    plugin->cbs.init = ryzen_plugin_init;
    plugin->cbs.get_devices = ryzen_plugin_get_devices;
    plugin->cbs.get_energy_update_interval = ryzen_get_energy_update_interval;
    plugin->cbs.get_energy_max = ryzen_get_energy_max;
    plugin->cbs.get_energy_uj = ryzen_plugin_get_energy_uj;
    plugin->cbs.finalize = ryzen_plugin_finalize;
    plugin->data = NULL;
    plugin->name = name;

    return plugin;
}

int register_ryzen_plugin(void)
{
    Plugin *plugin = create_ryzen_plugin("RYZEN");
    ASSERT_OR_1(plugin);
    return EMA_register_plugin(plugin);
}
