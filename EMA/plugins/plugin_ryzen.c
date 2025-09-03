#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <EMA/core/device.h>
#include <EMA/core/overflow.h>
#include <EMA/core/plugin.h>
#include <EMA/core/registry.h>
#include <EMA/core/utils.h>
#include <EMA/utils/error.h>

#include <EMA/ext/libsmu/libsmu.h>

#define DEVICE_TYPE "cpu"
#define TEST_SMN_ADDR 0x50200

smu_obj_t obj;

/* Typedefs */
typedef struct
{
    DeviceArray devices;
} RyzenPluginData;

typedef struct
{
    char* fw_version;
} RyzenDeviceData;

/* Utils */

void free_ryzen_device_data(RyzenDeviceData* data)
{
    free(data->fw_version);
    free(data);
}

/* Plugin interface */
static
int ryzen_plugin_init(Plugin* plugin)
{
    unsigned int result;

    DeviceArray devices;
    devices.size = 0;
    devices.array = NULL;

    RyzenPluginData* p_data = malloc(sizeof(RyzenPluginData));
    p_data->devices = devices;

    plugin->data = p_data;

    if (getuid() != 0 && geteuid() != 0) {
        fprintf(stderr, "Program must be run as root.\n");
        return 1;
    }

    if (smu_init(&obj) != SMU_Return_OK) {
        fprintf(stderr, "Error initializing userspace library.\n");
        return 1;
    }

    const char* codename = smu_codename_to_str(&obj);
    const char* version = smu_get_fw_version(&obj);

    devices.size = 1;
    devices.array = malloc(sizeof(Device));

    RyzenDeviceData* d_data = malloc(sizeof(RyzenDeviceData));
    d_data->fw_version = strdup(version);

    Device* device = devices.array;
    device->data = d_data;
    device->plugin = plugin;
    device->name = strdup(codename);
    device->type = strdup(DEVICE_TYPE);
    // TODO: Think of a better source for uid...
    device->uid = strdup(codename);
    int ret = EMA_init_overflow(device);
    ASSERT_MSG_OR_1(!ret, "Failed to register overflow handling.");

    if (smu_read_smn_addr(&obj, TEST_SMN_ADDR, &result) != SMU_Return_OK) {
        fprintf(stderr, "Error reading SMN address: 0x%08x\n", TEST_SMN_ADDR);
        exit(-3);
    }
    printf("SMN [0x%08x]: 0x%08x\n", TEST_SMN_ADDR, result);


    return 0;
}

static
DeviceArray ryzen_plugin_get_devices(const Plugin* plugin)
{
    RyzenPluginData* p_data = plugin->data;
    return p_data->devices;
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

    RyzenPluginData* p_data = (RyzenPluginData*) plugin->data;
    DeviceArray devices = p_data->devices;
    if (devices.array)
    {
        EMA_finalize_overflow(&devices.array[0]);
        free((void*)devices.array[0].name);
        free((void*)devices.array[0].type);
        free((void*)devices.array[0].uid);
        free_ryzen_device_data(devices.array[0].data);
    }

    free(devices.array);
    free(p_data);

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
