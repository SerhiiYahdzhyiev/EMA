#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>  // isdigit
#include <dirent.h>  // dirent, opendir(), readdir, closedir()
#include <errno.h>
#include <linux/limits.h>
#include <unistd.h>  // access, F_OK, R_OK

#include <EMA/core/device.h>
#include <EMA/core/overflow.h>
#include <EMA/core/plugin.h>
#include <EMA/core/registry.h>
#include <EMA/core/utils.h>
#include <EMA/utils/error.h>

#define RAPL_MAX 128
#define MAX_POWER_CONSTRAINTS 3

#define CONCAT(var, format, ...) \
    snprintf(var, PATH_MAX, format, ##__VA_ARGS__)

#define _RAPL_HANDLE_ERR(ret, msg, ...) do { \
    fprintf(stderr , msg, ##__VA_ARGS__); \
    return ret; \
} while(0)

#define RAPL_HANDLE_ERR(ret, msg, ...) \
    _RAPL_HANDLE_ERR(ret, msg,  ##__VA_ARGS__)

/* ****************************************************************************
**** Typedefs
**************************************************************************** */
typedef struct
{
    DeviceArray devices;
} RaplPluginData;

typedef struct
{
    void* zone;
    char* name;
    char* energy_zone;
    unsigned long long max_range;
    unsigned long long energy_update_interval_ms;
    int package;
    int has_read_perm;
} RaplDeviceData;

/* ****************************************************************************
**** Rapl Driver
**************************************************************************** */

static
int create_zone(int zone_idx, char* zone)
{
    int ret = CONCAT(
        zone,
        "/sys/class/powercap/intel-rapl/intel-rapl:%d",
        zone_idx
    );

    if(ret < 0)
        RAPL_HANDLE_ERR(1, "Creating zone string failed.\n");

    return 0;
}

static
int create_subzone(int zone_idx, int subzone_idx, char* subzone)
{
    int ret = CONCAT(
        subzone,
        "/sys/class/powercap/intel-rapl/intel-rapl:%d/intel-rapl:%d:%d",
        zone_idx,
        zone_idx,
        subzone_idx
    );

    if(ret < 0)
        RAPL_HANDLE_ERR(1, "Creating subzone string failed.\n");

    return 0;
}

static
int _count_dir_with_prefix(const char *path, const char* prefix)
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir(path);
    if( dir != NULL )
    {
        while( entry = readdir(dir) )
            if( strncmp(prefix, entry->d_name, strlen(prefix)) == 0 )
                ++count;
        closedir(dir);
    }
    else
        fprintf(stderr, "Failed to open the directory.\n");

    return count;
}

static
int get_num_rapl_sub_zones(int zone_idx)
{
    char zone[PATH_MAX];
    create_zone(zone_idx, zone);
    return _count_dir_with_prefix(zone, "intel-rapl");
}

static
int get_num_rapl_zones(void)
{
    return _count_dir_with_prefix(
        "/sys/class/powercap/intel-rapl", "intel-rapl");
}

static
int count_all_rapl_zones()
{
    int rapl_count = 0;
    for(int i = 0; i < get_num_rapl_zones(); i++)
    {
        rapl_count++;
        for(int j = 0; j < get_num_rapl_sub_zones(i); j++)
            rapl_count++;
    }
    return rapl_count;
}

static
int _file_exist(const char* fname)
{
    return access(fname, F_OK) == 0;
}

static
int _file_has_read_perm(const char* fname)
{
    return access(fname, R_OK) == 0;
}

/**
 * It is expected that the file is present and readable.
 * Return 1 on failure.
 */
static
int _read_rapl_file(const char* fname, char* value)
{
    FILE* ptr = fopen(fname, "r");
    if( ptr == NULL )
        RAPL_HANDLE_ERR(-1, "Failed to read RAPL value.\n");

    if( fgets(value, RAPL_MAX, ptr) == NULL )
    {
        fclose(ptr);
        RAPL_HANDLE_ERR(-1, "Failed to read RAPL value.\n");
    }

    fclose(ptr);
    return 0;
}

static
unsigned long long _read_rapl_max_range(const char* zone)
{
    char max_range_zone[PATH_MAX];
    char value_s[RAPL_MAX];

    CONCAT(max_range_zone, "%s%s", zone, "/max_energy_range_uj");

    int err = _read_rapl_file(max_range_zone, value_s);
    if( err != 0 )
        RAPL_HANDLE_ERR(0, "Could not read RAPL max_range.\n");

    errno = 0;
    unsigned long long value = strtoull(value_s, NULL, 10);
    if( errno != 0 )
        RAPL_HANDLE_ERR(
            0, "Failed to convert RAPL max_range: %s\n", strerror(errno));

    return value;
}

static
unsigned long long _read_rapl_constraint_max_power_uw(const char* zone)
{
    // constraint_0, constraint_1, constraint_2 - long_term, short_term, peak
    // No fixed mapping in general.
    //      e.g. constraint_0, constraint_1 - long_term, peak

    char max_power_zone[PATH_MAX];
    char value_s[RAPL_MAX];
    unsigned long long max_val = 0;

    for(int i = 0; i < MAX_POWER_CONSTRAINTS; i++)
    {
        CONCAT(
            max_power_zone,
            "%s%s%d%s",
            zone,
            "/constraint_",
            i,
            "_max_power_uw"
        );

        if( !_file_exist(max_power_zone) )
            continue;

        int err = _read_rapl_file(max_power_zone, value_s);
        if( err != 0 )
            RAPL_HANDLE_ERR(0, "Could not read RAPL max_range.\n");

        errno = 0;
        unsigned long long value = strtoull(value_s, NULL, 10);
        if( errno != 0 )
            RAPL_HANDLE_ERR(
                0,
                "Failed to convert RAPL constraint max_power_uw: %s\n",
                strerror(errno)
            );
        max_val = value > max_val ? value : max_val;
    }

    return max_val;
}

/**
 * Return -1 on internal failure,
 *         0 on success,
 *         1 if name file is missing,
 *         2 if name file has no read permission.
 */
static
int _read_zone_name(const char *zone, char* name)
{
    char name_zone[PATH_MAX];
    int ret = CONCAT(name_zone, "%s%s", zone, "/name");
    int err;

    if( ret < 0 )
        RAPL_HANDLE_ERR(-1, "Creating name-zone string failed.\n");

    if( !_file_exist(name_zone) )
        RAPL_HANDLE_ERR(1, "RAPL is not supported.\n");

    if( !_file_has_read_perm(name_zone) )
        RAPL_HANDLE_ERR(2, "No read permissions for %s.\n", name_zone);

    err = _read_rapl_file(name_zone, name);

    /* Remove newline character. */
    name[strcspn(name, "\n")] = '\0';
    return err;
}

/**
 * Return -1 if name has another format than "package-<id>".
 */
static
int _read_package_id(const char* name)
{
    char* str = strdup(name);
    char* token = strtok(str, "-");
    while( token != NULL )
    {
        if( strcmp(token, "package") == 0 ) // current token is "package"
        {
            token = strtok(NULL, "-"); // get target token: id
            if( token == NULL )
                goto cleanup;

            /* Check for number. */
            int is_number = 1;
            for(int i = 0; i < strlen(token); i++)
                if( !isdigit(token[i]) )
                    is_number = 0;

            if( is_number )
            {
                int id = atoi(token);
                free(str);
                return id; // return package_id
            }
            goto cleanup; // return -1 otherwise
        }
        token = strtok(NULL, "-");
    }

cleanup:
    free(str);
    return -1;
}

static
RaplDeviceData *create_rapl_device(const char* zone, const char* sub_zone)
{
    char name[RAPL_MAX];
    char energy_zone[PATH_MAX];
    unsigned long long max_range, constraint_max_power;
    unsigned long long energy_update_interval_ms;
    int ret;

    /* Prepare energy zone. */
    ret = CONCAT(
        energy_zone, "%s%s", sub_zone ? sub_zone : zone, "/energy_uj");

    if( ret < 0 )
        RAPL_HANDLE_ERR(NULL, "Creating energy-zone string failed.\n");

    if( !_file_exist(energy_zone) )
        RAPL_HANDLE_ERR(
            NULL, "RAPL is not supported. Missing: %s.\n", energy_zone);

    int has_read_perm = _file_has_read_perm(energy_zone);
    if( !has_read_perm )
        RAPL_HANDLE_ERR(
            NULL, "No read permissions for %s.\n", energy_zone);

    /* Prepare zone name and package id. */
    if( _read_zone_name(zone, name) != 0 )
        return NULL;

    int package_id = _read_package_id(name);
    if( sub_zone )
    {
        if( _read_zone_name(sub_zone, name) != 0 )
            return NULL;
    }

    /* Prepare rapl_device. */
    max_range = _read_rapl_max_range(zone);
    if( max_range == 0 )
        return NULL;

    constraint_max_power = _read_rapl_constraint_max_power_uw(zone);
    energy_update_interval_ms = 1000 * max_range / constraint_max_power;

    /* Setup rapl device. */
    RaplDeviceData *rapl_device = malloc(sizeof(RaplDeviceData));
    rapl_device->package = package_id;
    rapl_device->zone = sub_zone ? strdup(sub_zone) : strdup(zone);
    rapl_device->energy_zone = strdup(energy_zone);
    rapl_device->name = strdup(name);
    rapl_device->max_range = max_range;
    rapl_device->energy_update_interval_ms = energy_update_interval_ms;
    rapl_device->has_read_perm = has_read_perm;

    return rapl_device;
}

static
void free_rapl_device(RaplDeviceData *rapl_device)
{
    free(rapl_device->zone);
    free(rapl_device->energy_zone);
    free(rapl_device->name);
    free(rapl_device);
}

static
int get_full_name(char *full_name, int package, const char *name)
{
    int ret = snprintf(full_name, RAPL_MAX, "CPU-%d.%s", package, name);
    if( ret < 0 || ret >= RAPL_MAX )
        RAPL_HANDLE_ERR(1, "Failed to create RAPL device name.\n");
    return 0;
}


/* ****************************************************************************
**** Plugin interface
**************************************************************************** */

static
int rapl_plugin_init(Plugin* plugin)
{
    /* Create array of max possible size. */
    DeviceArray devices;
    devices.size = count_all_rapl_zones();
    devices.array = malloc(sizeof(Device) * devices.size);

    /* Iterate rapl top level zones. */
    int count_devices = 0;
    for(int i = 0; i < get_num_rapl_zones(); ++i)
    {
        RaplDeviceData *rapl_device;
        char zone[PATH_MAX];

        if( create_zone(i, zone) != 0 )
            continue;

        rapl_device = create_rapl_device(zone, NULL);
        if( !rapl_device )
            continue;

        /* Skip non accessable or cpu-related RAPL devices. */
        if( rapl_device->package < 0 )
        {
            free_rapl_device(rapl_device);
            continue;
        }

        char name[RAPL_MAX];
        int ret = get_full_name(
            name, rapl_device->package, rapl_device->name);
        if( ret != 0 )
        {
            free_rapl_device(rapl_device);
            continue;
        }

        Device *device = devices.array + count_devices++;
        device->data = rapl_device;
        device->plugin = plugin;
        device->name = strdup(name);
        ret = EMA_init_overflow(device);
        ASSERT_MSG_OR_1(!ret, "Failed to register overflow handling.");

        /* Iterate rapl sub zones. */
        for(int j = 0; j < get_num_rapl_sub_zones(i); ++j)
        {
            RaplDeviceData *rapl_sub_device;
            if( create_subzone(i, j, zone) != 0 )
                continue;

            rapl_sub_device = create_rapl_device(rapl_device->zone, zone);
            if( !rapl_sub_device )
                continue;

            /* Skip non accessable rapl_sub_devices */
            if( rapl_sub_device->package < 0 )
            {
                free_rapl_device(rapl_sub_device);
                continue;
            }

            char name[RAPL_MAX];
            int ret = get_full_name(
                name, rapl_sub_device->package, rapl_sub_device->name);
            if( ret != 0 )
            {
                free_rapl_device(rapl_sub_device);
                continue;
            }

            Device *device = devices.array + count_devices++;
            device->data = rapl_sub_device;
            device->plugin = plugin;
            device->name = strdup(name);
            ret = EMA_init_overflow(device);
            ASSERT_MSG_OR_1(!ret, "Failed to register overflow handling.");
        }
    }

    /* Shrink array to finally needed size. */
    devices.size = count_devices;
    devices.array = realloc_s(devices.array, sizeof(Device) * devices.size);

    /* Set plugin data. */
    RaplPluginData* p_data = malloc(sizeof(RaplPluginData));
    p_data->devices = devices;

    plugin->data = p_data;

    /* No access to RAPL devices. */
    if( count_devices == 0 && count_all_rapl_zones() > 0 )
        RAPL_HANDLE_ERR(1, "No access to RAPL devices.\n");

    /* No RAPL devices available. */
    if( count_devices == 0 )
        RAPL_HANDLE_ERR(0, "No RAPL devices detected.\n");

    return 0;
}

static
DeviceArray rapl_plugin_get_devices(const Plugin* plugin)
{
    RaplPluginData* p_data = plugin->data;
    return p_data->devices;
}

static
unsigned long long rapl_get_energy_update_interval(const Device* device)
{
    RaplDeviceData* d_data = device->data;
    return d_data->energy_update_interval_ms;
}

static
unsigned long long rapl_get_energy_max(const Device* device)
{
    RaplDeviceData* d_data = device->data;
    return d_data->max_range;
}

static
unsigned long long rapl_plugin_get_energy_uj(const Device* device)
{
    /*
    * Read energy value of the executing CPU.
    * Return measurement value.
    */
    char energy_str[RAPL_MAX];
    RaplDeviceData* d_data = device->data;

    /* Existence of the file and read permission already checked. */
    int err = _read_rapl_file(d_data->energy_zone, energy_str);
    if( err != 0 )
        RAPL_HANDLE_ERR(0, "Could not read RAPL energy value.\n");

    errno = 0;
    unsigned long long energy = strtoull(energy_str, NULL, 10);
    if( errno != 0 )
        RAPL_HANDLE_ERR(
            0, "Failed to convert RAPL energy: %s\n", strerror(errno));

    return energy;
}

static
int rapl_plugin_finalize(Plugin* plugin)
{
    RaplPluginData* p_data = (RaplPluginData*) plugin->data;
    DeviceArray devices = p_data->devices;
    for(int i = 0; i < devices.size; i++)
    {
        EMA_finalize_overflow(&devices.array[i]);
        free((void*)devices.array[i].name);
        free_rapl_device(devices.array[i].data);
    }
    free(devices.array);
    free(p_data);

    return 0;
}

/* ****************************************************************************
**** Extern
**************************************************************************** */

Plugin* create_rapl_plugin(const char* name)
{
    Plugin* plugin = malloc(sizeof(Plugin));
    ASSERT_OR_NULL(plugin);

    plugin->cbs.init = rapl_plugin_init;
    plugin->cbs.get_devices = rapl_plugin_get_devices;
    plugin->cbs.get_energy_update_interval = rapl_get_energy_update_interval;
    plugin->cbs.get_energy_max = rapl_get_energy_max;
    plugin->cbs.get_energy_uj = rapl_plugin_get_energy_uj;
    plugin->cbs.finalize = rapl_plugin_finalize;
    plugin->data = NULL;
    plugin->name = name;

    return plugin;
}

int register_rapl_plugin()
{
    Plugin *plugin = create_rapl_plugin("RAPL");
    ASSERT_OR_1(plugin);
    return EMA_register_plugin(plugin);
}
