#include <cpuid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <EMA/core/device.h>
#include <EMA/core/overflow.h>
#include <EMA/core/plugin.h>
#include <EMA/core/registry.h>
#include <EMA/core/utils.h>
#include <EMA/utils/error.h>

#include <EMA/ext/libsmu/libsmu.h>
#include <EMA/plugins/plugin_ryzen.h>

#define DEVICE_TYPE "cpu"

/* Typedefs */

typedef enum {
    PACKAGE,
    CORE
} ryzen_dev_type;

typedef struct
{
    smu_obj_t* smu;
    DeviceArray devices;
} RyzenPluginData;

typedef struct
{
    int core_idx;
    ryzen_dev_type type;
    unsigned long long energy_uj;
    struct timespec ts;
} RyzenDeviceData;

/* Utils */

static
int _get_fuse_topology(
    smu_obj_t* smu,
    int fam,
    int model,
    unsigned int* ccds_enabled,
    unsigned int* ccds_disabled,
    unsigned int* cores_disabled,
    unsigned int* smt_enabled
) {
    unsigned int ccds_down,
                 ccds_present,
                 core_fuse,
                 core_fuse_addr,
                 ccd_fuse1,
                 ccd_fuse2;

    ccd_fuse1 = 0x5D218;
    ccd_fuse2 = 0x5D21C;

    if (fam == 0x17 && model != 0x71) {
        ccd_fuse1 += 0x40;
        ccd_fuse2 += 0x40;
    }

    if (
        smu_read_smn_addr(smu, ccd_fuse1, &ccds_present) != SMU_Return_OK ||
        smu_read_smn_addr(smu, ccd_fuse2, &ccds_down) != SMU_Return_OK
    ) {
        fprintf(stdout, "error: failed to read CCD fuses");
        return 1;
    }

    *ccds_disabled = ((ccds_down & 0x3F) << 2) | ((ccds_present >> 30) & 0x3);

    ccds_present = (ccds_present >> 22) & 0xFF;
    *ccds_enabled = ccds_present;

    if (fam == 0x19)
        core_fuse_addr = (0x30081800 + 0x598) |
            ((((*ccds_disabled & ccds_present) & 1) == 1) ? 0x2000000 : 0);
    else
        core_fuse_addr =
            (0x30081800 + 0x238) | (((ccds_present & 1) == 0) ? 0x2000000 : 0);

    if (smu_read_smn_addr(smu, core_fuse_addr, &core_fuse) != SMU_Return_OK) {
        fprintf(stdout, "error: failed to read core fuse");
        return 1;
    }

    *cores_disabled = core_fuse & 0xFF;
    *smt_enabled = (core_fuse & (1 << 8)) != 0;
    return 0;
}

static
unsigned int _get_cores_count(smu_obj_t* smu)
{
    unsigned int ccds_enabled,
                 ccds_disabled,
                 core_disable_map,
                 logical_cores,
                 smt,
                 fam,
                 model,
                 eax,
                 ebx,
                 ecx,
                 edx;

    __get_cpuid(0x00000001, &eax, &ebx, &ecx, &edx);
    fam = ((eax & 0xf00) >> 8) + ((eax & 0xff00000) >> 20);
    model = ((eax & 0xf0000) >> 12) + ((eax & 0xf0) >> 4);
    logical_cores = (ebx >> 16) & 0xFF;

    int err = _get_fuse_topology(
        smu, fam, model, &ccds_enabled, &ccds_disabled,
        &core_disable_map, &smt
    );

    if (err) return -1;

    unsigned int cores = logical_cores;
    if (smt) cores /= 2;

    return cores;
}

/* Plugin interface */
static
int ryzen_plugin_init(Plugin* plugin)
{
    unsigned int result;
    smu_obj_t* smu = malloc(sizeof(smu_obj_t));

    DeviceArray devices;
    devices.size = 0;
    devices.array = NULL;

    RyzenPluginData* p_data = malloc(sizeof(RyzenPluginData));
    p_data->devices = devices;
    p_data->smu = smu;

    plugin->data = p_data;

    if (getuid() != 0 && geteuid() != 0)
    {
        fprintf(stderr, "Program must be run as root.\n");
        return 1;
    }

    smu_return_val ret = smu_init(smu);
    if (ret != SMU_Return_OK)
    {
        fprintf(
            stderr,
            "Error initializing userspace library.\n",
            smu_return_to_str(ret)
        );
        return 1;
    }

    if (!smu_pm_tables_supported(smu))
    {
        fprintf(stderr, "Error: pm_tabes are not supported.\n");
        return 1;
    }

    const char* codename = smu_codename_to_str(smu);

    unsigned int cores = _get_cores_count(smu);
    int total_devs = cores != 1 ? cores + 2 : 2;

    p_data->devices.size = total_devs;
    p_data->devices.array = malloc(sizeof(Device) * total_devs);

    char package_device_name[128];
    char core_device_name[128];

    sprintf(package_device_name, "%s.package", codename);
    sprintf(core_device_name, "%s.core", codename);

    RyzenDeviceData* d_data = malloc(sizeof(RyzenDeviceData));
    d_data->energy_uj = 0;
    d_data->core_idx = -1;
    d_data->type = PACKAGE;
    clock_gettime(CLOCK_MONOTONIC, &d_data->ts);

    Device* device = p_data->devices.array + 0;
    device->data = d_data;
    device->plugin = plugin;
    device->name = strdup(package_device_name);
    device->type = strdup(DEVICE_TYPE);
    device->uid = strdup(codename);

    d_data = malloc(sizeof(RyzenDeviceData));
    d_data->energy_uj = 0;
    d_data->core_idx = -1;
    d_data->type = CORE;
    clock_gettime(CLOCK_MONOTONIC, &d_data->ts);

    device = p_data->devices.array + 1;
    device->data = d_data;
    device->plugin = plugin;
    device->name = strdup(core_device_name);
    device->type = strdup(DEVICE_TYPE);
    device->uid = strdup(codename);

    if (total_devs == 2) return 0;

    for (int i = 2; i < total_devs; i++)
    {
        RyzenDeviceData* d_data = malloc(sizeof(RyzenDeviceData));
        d_data->energy_uj = 0;
        d_data->core_idx = i - 2;
        d_data->type = CORE;

        char name[128];
        sprintf(name, "%s.core.%d", codename, i - 2);

        device = p_data->devices.array + i;
        device->data = d_data;
        device->plugin = plugin;
        device->name = strdup(name);
        device->type = strdup(DEVICE_TYPE);
        device->uid = strdup(codename);
    }

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
    return 0;
}

static
unsigned long long ryzen_get_energy_max(const Device* device)
{
    return 0;
}

static
unsigned long long ryzen_plugin_get_energy_uj(const Device* device)
{
    RyzenDeviceData* d_data = (RyzenDeviceData*) device->data;
    RyzenPluginData* p_data = device->plugin->data;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double delta_us = 0;
    if (now.tv_sec > d_data->ts.tv_sec ||
        (now.tv_sec == d_data->ts.tv_sec && now.tv_nsec > d_data->ts.tv_nsec)) {
        delta_us = (now.tv_sec - d_data->ts.tv_sec) * 1000000ULL +
                   (now.tv_nsec - d_data->ts.tv_nsec) / 1000ULL;
    } else {
        return d_data->energy_uj;
    }
    d_data->ts = now;

    unsigned int table_size  = p_data->smu->pm_table_size;

    unsigned char* pm_buf = calloc(table_size, sizeof(unsigned char));
    ppm_table_0x240903 pmt = (ppm_table_0x240903)pm_buf;

    smu_return_val ret = smu_read_pm_table(p_data->smu, pm_buf, table_size);
    if (ret != SMU_Return_OK)
    {
        fprintf(
            stdout,
            "failed to read pm_table: %s\n",
            smu_return_to_str(ret)
        );
        free(pm_buf);
        return d_data->energy_uj;
    }

    double power;
    switch(d_data->type)
    {
        case PACKAGE:
            power = (double)pmt->SOCKET_POWER;
            break;
        case CORE:
            if (d_data->core_idx < 0)
            {
                power = pmt->VDDCR_CPU_POWER;
            }
            else
            {
                power = pmt->CORE_POWER[d_data->core_idx];
            }
            break;
    }

    unsigned long long energy_uj = (unsigned long long)(power * delta_us);
    d_data->energy_uj += energy_uj;

    free(pm_buf);

    return d_data->energy_uj;
}

static
int ryzen_plugin_finalize(Plugin* plugin)
{
    RyzenPluginData* p_data = (RyzenPluginData*) plugin->data;
    DeviceArray devices = p_data->devices;

    smu_free(p_data->smu);
    free(p_data->smu);

    if (devices.array)
    {
        for (int i = 0; i < devices.size; i++)
        {
            free((void*)devices.array[i].name);
            free((void*)devices.array[i].type);
            free((void*)devices.array[i].uid);
            free(devices.array[i].data);
        }
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
