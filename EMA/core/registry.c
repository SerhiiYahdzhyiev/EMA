#include <stdlib.h>

#include <EMA/utils/error.h>

#include "registry.h"

PluginRegistry registry = {
    .plugins = NULL,
    .devices = NULL
};

int EMA_register_plugin(Plugin* plugin)
{
    Plugin **mem = realloc(
        registry.plugins.array, (registry.plugins.size + 1) * sizeof(Plugin));
    ASSERT_OR_1(mem);

    registry.plugins.array = mem;
    registry.plugins.array[registry.plugins.size] = plugin;
    ++registry.plugins.size;
    return 0;
}
