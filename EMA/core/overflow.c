#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>

#include "device.h"
#include "plugin.h"
#include "registry.h"

#define ASSERT_1(expr, msg) if (!(expr)) { perror(msg); return 1; }

pthread_t overflow_handler;

static
int init_mutex(pthread_mutex_t *mutex)
{
    int ret;

    ret = pthread_mutex_init(mutex, NULL);
    ASSERT_1(!ret, "Failed to run `pthread_mutex_init()`.");

    return 0;
}

static
int destroy_mutex(pthread_mutex_t *mutex)
{
    int ret;

    ret = pthread_mutex_destroy(mutex);
    ASSERT_1(!ret, "Failed to run `pthread_mutex_destroy()`.");

    return 0;
}

static
void* track_overflows(void* args)
{
    DevicePtrArray* dev_ptrs = args;

    uint64_t min_interval = UINT64_MAX;
    for(int i = 0; i < dev_ptrs->size; i++)
    {
        Device* device = dev_ptrs->array[i];
        uint64_t interval =
            device->plugin->cbs.get_energy_update_interval(device);

        if( interval > 0 && interval < min_interval )
                min_interval = interval;
    }

    while( 1 )
    {
        for(int i = 0; i < dev_ptrs->size; i++)
        {
            Device* device = dev_ptrs->array[i];
            uint64_t interval =
                device->plugin->cbs.get_energy_update_interval(device);

            if( interval == 0 )
                continue;

            unsigned long long cur_e;
            OverflowData* ofd = &device->overflow;

            /* Thread safe! Only single writer, no readers allowed. */
            pthread_mutex_lock(&ofd->mutex);
            cur_e = device->plugin->cbs.get_energy_uj(device);
            if( device->overflow.old > cur_e )
                ofd->count++;
            ofd->old = cur_e;
            pthread_mutex_unlock(&ofd->mutex);
        }
        usleep((uint64_t) min_interval * 1000);
    }
}

int start_overflow_tracking(DevicePtrArray* dev_ptrs)
{
    int ret = pthread_create(
        &overflow_handler, NULL, &track_overflows, (void *) dev_ptrs);
    ASSERT_1(!ret, "Failed to start overflow_tracking.");
}

int stop_overflow_tracking()
{
    int ret = pthread_cancel(overflow_handler);
    ASSERT_1(!ret, "Failed to cancel overflow thread.");

    ret = pthread_join(overflow_handler, NULL);
    ASSERT_1(!ret, "Failed to join overflow thread.");

    return 0;
}

unsigned long long EMA_get_handled_energy_uj(const Device* device)
{
    PluginCallbacks cbs = device->plugin->cbs;
    const OverflowData* ofd = &device->overflow;

    /* Thread safe! Multiple readers allowed, no writers. */
    pthread_mutex_lock((pthread_mutex_t*)&ofd->mutex);
    unsigned long long old = ofd->old;
    unsigned long long count = ofd->count;
    unsigned long long cur_energy = cbs.get_energy_uj(device);
    pthread_mutex_unlock((pthread_mutex_t*)&ofd->mutex);

    if( old > cur_energy )
        count++;

    return count * cbs.get_energy_max(device) + cur_energy;
}

int EMA_init_overflow(Device* device)
{
    EMA_plugin_cb_get_energy_uj get_energy = device->plugin->cbs.get_energy_uj;
    device->overflow.count = 0;
    device->overflow.old = get_energy(device);
    int ret = init_mutex(&device->overflow.mutex);
    ASSERT_1(!ret, "Failed to run `init_mutex()`.");
    return 0;
}

int EMA_finalize_overflow(Device* device)
{
    int ret = destroy_mutex(&device->overflow.mutex);
    ASSERT_1(!ret, "Failed to run `destroy_mutex()`.");
    return 0;
}
