#include <stdlib.h>

#include "registry.h"

#define ASSERT(COND, RET) \
    if( !(COND) ) \
        return RET;

PluginRegistry registry = {
    .plugins = NULL,
    .devices = NULL
};

int EMA_register_plugin(Plugin* plugin)
{
    Plugin **mem = realloc(
        registry.plugins.array, (registry.plugins.size + 1) * sizeof(Plugin));
    ASSERT(mem, 1);

    registry.plugins.array = mem;
    registry.plugins.array[registry.plugins.size] = plugin;
    ++registry.plugins.size;
    return 0;
}
