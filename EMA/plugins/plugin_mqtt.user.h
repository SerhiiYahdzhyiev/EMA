#include <stdint.h>

#include <EMA/core/plugin.user.h>

typedef struct
{
    char* host;
    uint16_t port;
    char* topic;
    int read_devices_timeout_sec;
    int read_energy_timeout_sec;
} MqttPluginConfig;

Plugin* create_mqtt_plugin(const char* name, MqttPluginConfig* config);

int register_mqtt_plugin(const char* name, MqttPluginConfig* config);
