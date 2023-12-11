#ifndef EMA_USER_H
#define EMA_USER_H

#include <threads.h>

#include <EMA/core/device.user.h>
#include <EMA/core/plugin.user.h>
#include <EMA/region/output.user.h>
#include <EMA/region/region.user.h>
#include <EMA/utils/time.user.h>

/**
 * Type definition of the initialization callback.
 */
typedef int (*EMA_init_cb)(void);

/* Initialization and cleanup. */

/**
 * This function initializes EMA framework. `Plugins` and `Devices` will be
 * added to the registry.
 *
 * @param EMA_init_cb: A callback function to modify the initialization of EMA.
 * It provides a possibility to modify the registration of `Plugins` and
 * `Devices`.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_init(EMA_init_cb);

/**
 * This function finalizes the EMA framework.
 */
int EMA_finalize();

/* Plugin interface. */

/**
 * This function initializes a given plugin.
 *
 * @param plugin: `Plugin` to initialize.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_plugin_init(Plugin* plugin);

/**
 * This function collects all `Devices` of a given `Plugin`.
 *
 * @param plugin: `Plugin` which provides the `Devices` to collect.
 *
 * @returns The `Devices` of the given `Plugin`.
 */
DeviceArray EMA_get_plugin_devices(const Plugin* plugin);

/**
 * This function reads the current energy value of a given `Device`.
 *
 * @param device: `Device` from which the energy value is to be read.
 *
 * @returns The current energy value in micro joules of the given `Device`.
 */
unsigned long long EMA_plugin_get_energy_uj(const Device* device);

/**
 * This function finalizes a given `Plugin`.
 *
 * @param plugin: `Plugin` which is to be finalized.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_plugin_finalize(Plugin* plugin);

/* Plugin interface. */

/**
 * This function reads the name of a given `Plugin`.
 *
 * @param plugin: `Plugin` from which the name is to be read.
 *
 * @returns The name of the given `Plugin`.
 */
const char* EMA_get_plugin_name(const Plugin* plugin);

/* Device interface. */

/**
 * This function reads the name of a given `Device`.
 *
 * @param device: `Device` from which the name is to be read.
 *
 * @returns The name of the given `Device`.
 */
const char* EMA_get_device_name(const Device* device);

/* Global interface. */

/**
 * This function collects the registered `Plugins`.
 *
 * @returns The registered `Plugins`.
 */
PluginPtrArray EMA_get_plugins();

/**
 * This function collects the registered `Devices`.
 *
 * @returns The registered `Devices`.
 */
DevicePtrArray EMA_get_devices();

#endif
