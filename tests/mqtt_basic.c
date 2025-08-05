#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include <unistd.h>

#include <EMA.h>
#include <EMA/plugins/plugin_mqtt.user.h>

/* !!! Adjust these values to match your mosquitto setup !!! */
#define MQTT_PLUGIN_NAME "mqtt-plugin-0"
#define MQTT_HOST "127.0.0.1"
#define MQTT_PORT 1883
#define MQTT_TOPIC "EMA/hhi-mqtt-adapter/0/publish"

int main(int argc, char **argv)
{
    MqttPluginConfig config = {
        .host = MQTT_HOST,
        .port = MQTT_PORT,
        .topic = MQTT_TOPIC,
        .read_devices_timeout_sec = 5,
        .read_energy_timeout_sec = 1
    };

    int err = register_mqtt_plugin(
        MQTT_PLUGIN_NAME,
        &config
    );
    if (err) {
       printf("Error: %d\n", err);
       return 1;
    }

    printf("Initializing...\n");
    err = EMA_init(NULL);
    if( err )
    {
        printf("Error: %d\n", err);
        return 1;
    }

    PluginPtrArray plugins = EMA_get_plugins();
    printf("Num plugins: %lu\n", plugins.size);
    for(size_t i = 0; i < plugins.size; ++i) {
        printf("Plugin %lu: %s\n", i, EMA_get_plugin_name(plugins.array[i]));
        DevicePtrArray devices = EMA_get_plugin_devices(plugins.array[i]);
        if (devices.size) {
            printf("\tPlugin devices: %lu\n", devices.size);
            for(size_t j = 0; j < devices.size; ++j)
                printf("\t\tDevice %lu: %s\n", j, EMA_get_device_name(devices.array[j]));
        } else {
            printf("\tPlugin devices: %lu\n", devices.size);
            printf("\t\tFailed to obtain devices for plugin!\n");
        }
        free(devices.array);
    }

    /* Filter. */
    Filter *filter = EMA_filter_exclude_plugin("NVML");

    printf("Preparing region...\n");
    EMA_REGION_DECLARE(region);
    EMA_REGION_DEFINE_WITH_FILTER(&region, "r0", filter);

    printf("Starting region measurements...\n");
    EMA_REGION_BEGIN(region);

    sleep(2);

    printf("Stoping region measurements...\n");
    EMA_REGION_END(region);

    EMA_filter_finalize(filter);

    printf("Output\n");
    EMA_print_all(stdout);

    printf("Finalizing...\n");
    err = EMA_finalize();
    if (err) {
        printf("Error: EMA_finalize: %d:", err);
        return 1;
    }

    return 0;
}
