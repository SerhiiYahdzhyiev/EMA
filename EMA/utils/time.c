#include "time.h"

#define TIME_US(ts) ((ts).tv_sec * 1000000ULL + (ts).tv_nsec / 1000ULL);

/**
 * This function returns the current time.
 * @return Current time in micro seconds.
 */
unsigned long long EMA_get_time_in_us()
{
    /* Return current time in microseconds. */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return TIME_US(ts);
}
