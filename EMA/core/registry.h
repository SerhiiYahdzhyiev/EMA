#ifndef EMA_CORE_REGISTRY_H
#define EMA_CORE_REGISTRY_H

#include "device.h"
#include "plugin.h"

typedef struct
{
    PluginPtrArray plugins;
    DevicePtrArray devices;
} PluginRegistry;

int EMA_register_plugin(Plugin* plugin);
void EMA_unregister_plugin(Plugin* plugin);

#endif
