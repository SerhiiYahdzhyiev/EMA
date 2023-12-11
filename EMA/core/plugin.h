#ifndef EMA_CORE_PLUGIN_H
#define EMA_CORE_PLUGIN_H

#include <stddef.h>

#include "device.h"
#include "plugin.user.h"

#define IS_PLUGIN_ACTIVATED(x) x && (!strcmp(x, "ON") || !strcmp(x, "1"))

typedef struct Plugin Plugin;

typedef int (*EMA_plugin_cb_init)(Plugin*);
typedef DeviceArray (*EMA_plugin_cb_get_devices)(const Plugin*);
typedef unsigned long long (*EMA_plugin_cb_get_energy_uj)(const Device*);
typedef int (*EMA_plugin_cb_finalize)(Plugin*);

typedef struct
{
    EMA_plugin_cb_init init;
    EMA_plugin_cb_get_devices get_devices;
    EMA_plugin_cb_get_energy_uj get_energy_uj;
    EMA_plugin_cb_finalize finalize;
} PluginCallbacks;

typedef struct Plugin
{
    PluginCallbacks cbs;
    const char *name;
    void *data;
} Plugin;

#endif
