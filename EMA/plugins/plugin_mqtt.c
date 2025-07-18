#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mosquitto.h>

#include <EMA/core/device.h>
#include <EMA/core/overflow.h>
#include <EMA/core/plugin.h>
#include <EMA/core/registry.h>
#include <EMA/utils/error.h>

#define VERSION 0x1

#define _MQTT_HANDLE_ERR(err, ret, format, ...) do { \
    if(err) { \
        fprintf(stderr, format, ##__VA_ARGS__); \
        return ret; \
    } \
} while(0)

#define MQTT_HANDLE_ERR(ret, msg, ...) \
    _MQTT_HANDLE_ERR(ret, msg,  ##__VA_ARGS__)

#define MQTT_HANDLE_ERR_RET_0(err, format, ...) \
    _MQTT_HANDLE_ERR(err, 0, format, ##__VA_ARGS__)

#define MQTT_HANDLE_ERR_RET_1(err, format, ...) \
    _MQTT_HANDLE_ERR(err, 1, format, ##__VA_ARGS__)

#define MQTT_HANDLE_ERR_RET_NULL(err, format, ...) \
    _MQTT_HANDLE_ERR(err, NULL, format, ##__VA_ARGS__)

/* TODO: Make this part of registration message, instead of hard-coding. */
#define DEVICE_TYPE "misc"

/* ****************************************************************************
**** Typedefs
**************************************************************************** */
typedef struct
{
    uint16_t size;
    uint8_t* bytes;
} ByteArray;

typedef struct
{
    uint64_t value;
    uint64_t time;
} MsrValues;

typedef struct
{
    const char* host;
    uint16_t port;
    const char* topic;
} MqttPluginConfig;

typedef struct
{
    DeviceArray devices;
    MqttPluginConfig* config;
} MqttPluginData;

typedef struct
{
    char* name;
    char* uid;
    char* topic;
    uint8_t type;
    MqttPluginConfig* config;
} MqttDeviceData;

/* ****************************************************************************
**** Utils
**************************************************************************** */
static
uint64_t byte_to_uint64(uint8_t* buf)
{
   return le64toh(*((uint64_t*) buf));
}

static
uint16_t byte_to_uint16(uint8_t* buf)
{
   return le16toh(*((uint16_t*) buf));
}

static
char* byte_to_char_ptr(uint8_t* buf, uint16_t len)
{
   return strndup(buf, len);
}

static
uint16_t bytes_to_msr_values(uint8_t* buf, MsrValues* msr)
{
    uint16_t offset = 0;

    msr->value = byte_to_uint64(&buf[offset]);
    offset += sizeof(uint64_t);

    msr->time = byte_to_uint64(&buf[offset]);
    offset += sizeof(uint64_t);

    return offset;
}

static
uint16_t bytes_to_mqtt_device_data(uint8_t* buf, MqttDeviceData* d_data)
{
    uint16_t offset = 0;

    size_t len = byte_to_uint64(&buf[offset]);
    offset += sizeof(uint64_t);

    d_data->name = byte_to_char_ptr(&buf[offset], len);
    d_data->uid = byte_to_char_ptr(&buf[offset], len);
    offset += len;

    len = byte_to_uint64(&buf[offset]);
    offset += sizeof(uint64_t);

    d_data->topic = byte_to_char_ptr(&buf[offset], len);
    offset += len;

    d_data->type = buf[offset++];

    return offset;
}

static
void on_message_read_bytes(
    struct mosquitto *mqtt, void *obj, const struct mosquitto_message *msg)
{
    if( !mqtt )
        return;

    ByteArray* bytes = obj;

    bytes->size = msg->payloadlen;
    bytes->bytes = malloc(bytes->size);
    memcpy(bytes->bytes, msg->payload, bytes->size);

    mosquitto_unsubscribe(mqtt, NULL, msg->topic);
    mosquitto_disconnect(mqtt);
}

static
int read_byte_message(
    struct mosquitto *mqtt, const char* host, uint16_t port, const char* topic)
{
    if( !mqtt )
        return 0;

    int ret = mosquitto_connect(mqtt, host, port, 5);
    MQTT_HANDLE_ERR_RET_1(
        ret, "Failed to connect to mosquitto: mosquitto_connect(): %d.\n", ret);

    ret = mosquitto_subscribe(mqtt, NULL, topic, 0);
    MQTT_HANDLE_ERR_RET_1(
        ret, "Failed to connect to mosquitto: mosquitto_subscribe().\n");

    ret = mosquitto_loop_start(mqtt);
    MQTT_HANDLE_ERR_RET_1(
        ret, "Failed to connect to mosquitto: mosquitto_loop_start().\n");

    ret = mosquitto_loop_stop(mqtt, false);
    MQTT_HANDLE_ERR_RET_1(
        ret, "Failed to connect to mosquitto: mosquitto_loop_stop().\n");

    return 0;
}

static
uint8_t* read_devices(MqttPluginConfig* config)
{
    ByteArray bytes;
    bytes.size = 0;
    bytes.bytes = NULL;

    struct mosquitto *mqtt = mosquitto_new(NULL, 1, &bytes);

    mosquitto_message_callback_set(mqtt, on_message_read_bytes);

    int err = read_byte_message(
        mqtt, config->host, config->port, config->topic);
    if (err) mosquitto_destroy(mqtt);
    MQTT_HANDLE_ERR_RET_NULL(err, "Failed to read byte message.\n");

    mosquitto_destroy(mqtt);

    return bytes.bytes;
}

static
uint64_t read_energy(MqttDeviceData* device)
{
    ByteArray bytes;
    bytes.size = 0;
    bytes.bytes = NULL;

    struct mosquitto *mqtt = mosquitto_new(NULL, 1, &bytes);
    mosquitto_message_callback_set(mqtt, on_message_read_bytes);

    int err = read_byte_message(
        mqtt,
        device->config->host,
        device->config->port,
        device->topic
    );
    if (err) mosquitto_destroy(mqtt);
    MQTT_HANDLE_ERR_RET_1(err, "Failed to read byte message.\n");

    mosquitto_destroy(mqtt);

    MsrValues msr;
    bytes_to_msr_values(bytes.bytes, &msr);

    free(bytes.bytes);
    return msr.value;
}

/* ****************************************************************************
**** Plugin interface
**************************************************************************** */

static
int mqtt_plugin_init(Plugin* plugin)
{
    DeviceArray devices;
    MqttPluginData* p_data = plugin->data;
    MqttPluginConfig* config = p_data->config;

    /* Initialize mosquitto. */
    mosquitto_lib_init();

    /* Set device count. */
    uint8_t* bytes = read_devices(config);
    MQTT_HANDLE_ERR_RET_1(bytes == NULL, "Failed to read devices.\n");

    /* Check version. */
    uint8_t* buf = bytes;
    if(VERSION != *buf)
    {
        fprintf(
            stderr, "Version mismatch!\n\t Required: %u\n\t Found: %u\n",
            VERSION, *buf);
        free(buf);
        return 1;
    }
    buf += sizeof(uint8_t);

    /* Init devices. */
    devices.size = byte_to_uint16(buf);
    buf += sizeof(uint16_t);

    devices.array = malloc(sizeof(Device)*devices.size);

    for(int i = 0; i < devices.size; i++)
    {
        MqttDeviceData* d_data = malloc(sizeof(MqttDeviceData));
        d_data->config = config;
        buf += bytes_to_mqtt_device_data(buf, d_data);
        printf("%s - %s\n", d_data->name, d_data->topic);

        /* Set device array. */
        devices.array[i].name = d_data->name;
        /* TODO: Derive the type from d_data as well. */
        devices.array[i].type = strdup(DEVICE_TYPE);
        devices.array[i].name = d_data->uid;
        devices.array[i].data = d_data;
        devices.array[i].plugin = plugin;

        /* Init overflow handling. */
        int ret = EMA_init_overflow(&devices.array[i]);
        ASSERT_MSG_OR_1(!ret, "Failed to register overflow handling.");
    }
    free(bytes);

    /* Set plugin data devices. */
    p_data->devices = devices;

    return 0;
}

static
DeviceArray mqtt_plugin_get_devices(const Plugin* plugin)
{
    MqttPluginData* p_data = plugin->data;
    return p_data->devices;
}

static
unsigned long long mqtt_plugin_get_energy_update_interval(const Device* device)
{
    return 0;
}

static
unsigned long long mqtt_plugin_get_energy_max(const Device* device)
{
    return ULLONG_MAX;
}

static
unsigned long long mqtt_plugin_get_energy_uj(const Device* device)
{
    return read_energy(device->data);
}

static
int mqtt_plugin_finalize(Plugin* plugin)
{
    MqttPluginData* p_data = (MqttPluginData*) plugin->data;
    DeviceArray devices = p_data->devices;
    for(size_t i = 0; i < devices.size; i++)
    {
        EMA_finalize_overflow(&devices.array[i]);
        MqttDeviceData* d_data = devices.array[i].data;
        free(d_data->name);
        free((void*)devices.array[i].type);
        free(d_data->uid);
        free(d_data->topic);
        free(d_data);
    }

    free(p_data->devices.array);
    free(p_data->config);
    free(p_data);

    mosquitto_lib_cleanup();
    return 0;
}

/* ****************************************************************************
**** Extern
**************************************************************************** */

Plugin* create_mqtt_plugin(
    const char* name, const char* host, uint16_t port, const char* topic)
{
    MqttPluginData* p_data = malloc(sizeof(MqttPluginData));
    MqttPluginConfig* config = malloc(sizeof(MqttPluginConfig));
    config->host = host;
    config->port = port;
    config->topic = topic;

    p_data->devices.array = NULL;
    p_data->devices.size = 0;
    p_data->config = config;

    Plugin* plugin = malloc(sizeof(Plugin));
    ASSERT_OR_NULL(plugin);

    plugin->cbs.init = mqtt_plugin_init;
    plugin->cbs.get_devices = mqtt_plugin_get_devices;
    plugin->cbs.get_energy_update_interval =
        mqtt_plugin_get_energy_update_interval;
    plugin->cbs.get_energy_max = mqtt_plugin_get_energy_max;
    plugin->cbs.get_energy_uj = mqtt_plugin_get_energy_uj;
    plugin->cbs.finalize = mqtt_plugin_finalize;
    plugin->data = p_data;
    plugin->name = name;

    return plugin;
}

int register_mqtt_plugin(
    const char* name, const char* host, uint16_t port, const char* topic)
{
    Plugin *plugin = create_mqtt_plugin(name, host, port, topic);
    ASSERT_OR_1(plugin);
    return EMA_register_plugin(plugin);
}
