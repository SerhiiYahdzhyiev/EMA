#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <nvml.h>

#include <EMA/core/device.h>
#include <EMA/core/overflow.h>
#include <EMA/core/plugin.h>
#include <EMA/core/registry.h>
#include <EMA/core/utils.h>
#include <EMA/utils/error.h>

#define _NVML_HANDLE_ERR(err, ret, format, ...) do { \
    if(err) { \
        fprintf(stderr, format, ##__VA_ARGS__); \
        nvml_shutdown(); \
        return ret; \
    } \
} while(0)

#define NVML_HANDLE_ERR(err, format, ...) \
    _NVML_HANDLE_ERR(err, , format, ##__VA_ARGS__)

#define NVML_HANDLE_ERR_RET_NULL(err, format, ...) \
    _NVML_HANDLE_ERR(err, NULL, format, ##__VA_ARGS__)

#define NVML_HANDLE_ERR_RET_0(err, format, ...) \
    _NVML_HANDLE_ERR(err, 0, format, ##__VA_ARGS__)

#define NVML_HANDLE_ERR_RET_1(err, format, ...) \
    _NVML_HANDLE_ERR(err, 1, format, ##__VA_ARGS__)

#define DEVICE_TYPE "gpu"

/* ****************************************************************************
**** Typedefs
**************************************************************************** */
typedef struct
{
    DeviceArray devices;
} NvmlPluginData;

typedef struct
{
    unsigned idx;
    nvmlDevice_t nvml_device;
    char* name;
    char* uid;
} NvmlDeviceData;

/* ****************************************************************************
**** Internal
**************************************************************************** */

static
void nvml_shutdown(void)
{
    nvmlReturn_t err = nvmlShutdown();
    if(err)
        printf("NVML shutdown failed: %s\n", nvmlErrorString(err));
}

static
NvmlDeviceData* init_nvml_device(unsigned idx)
{
    nvmlReturn_t err;

    /* Set NVML devices */
    NvmlDeviceData* d_data = malloc(sizeof(NvmlDeviceData));

    /* Set NVML device handle. */
    err = nvmlDeviceGetHandleByIndex(idx, &d_data->nvml_device);
    NVML_HANDLE_ERR_RET_NULL(
        err,
        "Failed to get device handle %u: %s\n",
        idx, nvmlErrorString(err)
    );

    /* Set NVML device name */
    char *device_name = malloc(NVML_DEVICE_NAME_V2_BUFFER_SIZE * sizeof(char));
    err = nvmlDeviceGetName(
        d_data->nvml_device,
        device_name,
        NVML_DEVICE_NAME_V2_BUFFER_SIZE
    );
    NVML_HANDLE_ERR_RET_NULL(
        err,
        "Failed to get name of device %u: %s\n",
        idx, nvmlErrorString(err)
    );
    d_data->name = device_name;

    /* Set NVML device uuid */
    char *device_uuid = malloc(NVML_DEVICE_UUID_BUFFER_SIZE * sizeof(char));
    err = nvmlDeviceGetUUID(
        d_data->nvml_device,
        device_uuid,
        NVML_DEVICE_UUID_BUFFER_SIZE
    );
    NVML_HANDLE_ERR_RET_NULL(
        err,
        "Failed to get UUID of device %u: %s\n",
        idx, nvmlErrorString(err)
    );
    d_data->uid = device_uuid;

    /* Set NVML device index */
    d_data->idx = idx;

    return d_data;
}

/* ****************************************************************************
**** Plugin interface
**************************************************************************** */
static
int nvml_plugin_init(Plugin* plugin)
{
    /* Init plugin data. */
    NvmlPluginData* p_data = malloc(sizeof(NvmlPluginData));
    p_data->devices.size = 0;
    p_data->devices.array = NULL;
    plugin->data = p_data;

    nvmlReturn_t err;
    DeviceArray devices;
    unsigned int k = 0; /* accessable device count */
    unsigned int device_count = 0;

    /* Initialize NVML. */
    err = nvmlInit();
    NVML_HANDLE_ERR_RET_1(
        err, "NVML initialization failed: %s\n", nvmlErrorString(err));

    /* Set device count. */
    err = nvmlDeviceGetCount(&device_count);
    NVML_HANDLE_ERR_RET_1(
        err, "Could not query device count: %s\n", nvmlErrorString(err));

    /* Init devices. */
    devices.size = device_count;
    devices.array = malloc(sizeof(Device)*devices.size);
    for(int i = 0; i < devices.size; i++)
    {
        NvmlDeviceData* d_data = init_nvml_device(i);
        if(d_data == NULL)
            continue;

        /* Set device array. */
        devices.array[k].data = d_data;
        devices.array[k].plugin = plugin;

        /* Keep default name on change. */
        devices.array[k].name = d_data->name;
        devices.array[k].uid = d_data->uid;

        devices.array[k].type = strdup(DEVICE_TYPE);

        /* Init overflow handling. */
        int ret = EMA_init_overflow(&devices.array[k]);
        NVML_HANDLE_ERR_RET_1(ret, "Failed to register overflow handling.");
        ++k;
    }

    /* Update device data. */
    devices.size = k;
    devices.array = realloc_s(devices.array, sizeof(Device) * devices.size);

    /* Set plugin data devices. */
    p_data->devices = devices;

    /* No access to NVML devices. */
    NVML_HANDLE_ERR_RET_1(
        k == 0 && device_count > 0, "No access to NVML devices.\n");

    /* No NVML devices available. */
    NVML_HANDLE_ERR_RET_0(device_count == 0, "No NVML devices detected.\n");

    return 0;
}

static
DeviceArray nvml_plugin_get_devices(const Plugin* plugin)
{
    NvmlPluginData* p_data = plugin->data;
    return p_data->devices;
}

static
unsigned long long nvml_plugin_get_energy_update_interval(const Device* device)
{
    return 0;
}

static
unsigned long long nvml_plugin_get_energy_max(const Device* device)
{
    return ULLONG_MAX;
}

static
unsigned long long nvml_plugin_get_energy_uj(const Device* device)
{
    /*
    * Read energy values of a single NVIDIA GPU, if available.
    * Return energy consumption in uJ.
    */
    nvmlReturn_t err;
    unsigned long long energy;
    NvmlDeviceData* d_data = (NvmlDeviceData*) device->data;

    /* Get energy consumption in mJ. */
    err = nvmlDeviceGetTotalEnergyConsumption(d_data->nvml_device, &energy);
    NVML_HANDLE_ERR_RET_0(
        err,
        "NVML - Failed to get total energy consumption of GPU %u: %s\n",
        d_data->idx, nvmlErrorString(err)
    );

    return energy * 1000;
}

static
int nvml_plugin_finalize(Plugin* plugin)
{
    NvmlPluginData* p_data = (NvmlPluginData*) plugin->data;
    DeviceArray devices = p_data->devices;
    for(int i = 0; i < devices.size; i++)
    {
        EMA_finalize_overflow(&devices.array[i]);
        NvmlDeviceData* d_data = devices.array[i].data;
        free((void*)devices.array[i].type);
        free(d_data->name);
        free(d_data->uid);
        free(d_data);
    }
    free(p_data->devices.array);
    free(p_data);

    nvml_shutdown();
    return 0;
}

/* ****************************************************************************
**** Extern
**************************************************************************** */
Plugin* create_nvml_plugin(const char* name)
{
    Plugin* plugin = malloc(sizeof(Plugin));
    ASSERT_OR_NULL(plugin);

    plugin->cbs.init = nvml_plugin_init;
    plugin->cbs.get_devices = nvml_plugin_get_devices;
    plugin->cbs.get_energy_update_interval =
        nvml_plugin_get_energy_update_interval;
    plugin->cbs.get_energy_max = nvml_plugin_get_energy_max;
    plugin->cbs.get_energy_uj = nvml_plugin_get_energy_uj;
    plugin->cbs.finalize = nvml_plugin_finalize;
    plugin->data = NULL;
    plugin->name = name;

    return plugin;
}

int register_nvml_plugin(void)
{
    Plugin *plugin = create_nvml_plugin("NVML");
    ASSERT_OR_1(plugin);
    return EMA_register_plugin(plugin);
}
