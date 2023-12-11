#ifndef EMA_REGION_REGION_H
#define EMA_REGION_REGION_H

#include <EMA/core/device.h>
#include "region.user.h"

typedef struct
{
    const Device *device;
    unsigned long long energy_start;
    unsigned long long energy_result;
    unsigned long long time_start;
    unsigned long long time_result;

} Measurement;

typedef struct
{
    Measurement* array;
    size_t size;
} MeasurementArray;

typedef struct Region
{
    /* measurement data */
    MeasurementArray measurements;
    unsigned long long visits;

    /* user info and hashkey. */
    char* idf;
    char* file;
    char* function;
    unsigned int line;
} Region;

#endif
