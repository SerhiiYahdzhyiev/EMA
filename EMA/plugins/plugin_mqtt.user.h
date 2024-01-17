#include <stdint.h>

#include <EMA/core/plugin.user.h>

Plugin* create_mqtt_plugin(
    const char* name, const char* host, uint16_t port, const char* topic);
int register_mqtt_plugin(
    const char* name, const char* host, uint16_t port, const char* topic);
