#ifndef EMA_CORE_OVERFLOW_H
#define EMA_CORE_OVERFLOW_H

#include <stdint.h>

#include <pthread.h>

#include "plugin.h"
#include "registry.h"

typedef struct OverflowData
{
    unsigned long long count;
    unsigned long long old;
    pthread_mutex_t mutex;
} OverflowData;

int start_overflow_tracking(DevicePtrArray* dev_ptrs);
int stop_overflow_tracking();
unsigned long long EMA_get_handled_energy_uj(const Device* device);
int EMA_init_overflow(Device* device);
int EMA_finalize_overflow(Device* device);

#endif
