#ifndef EMA_REGION_REGION_USER_H
#define EMA_REGION_REGION_USER_H

#include "filter.user.h"

/* Region interface. */
/**
 * The `Region` type stores data of a code segment that is to be measured.
 *
 * .. note::
 *    Operations on `Region` objects are not thread-safe. Thus, special
*     treatment is required in multi-threaded code.
 *
 */
typedef struct Region Region;

/**
 * This function creates and initializes a region.
 *
 * @param region: `Region` that should be created and initialized.
 * @param idf: Region identifier or name of the `Region`.
 * @param filter: `Filter` to define `Devices` for the measurement of the
 * `Region`.
 * @param file: File name that describes where the `Region` is defined.
 * @param line: Line number that describes where the `Region` is defined.
 * @param func: Function name that describes where the `Region` is defined.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_region_create_and_init(
    Region **region,
    const char* idf,
    Filter *filter,
    const char* file,
    unsigned int line,
    const char* func
);

/**
 * This function starts a measurement for a given region. Calling this function
 * multiple times overrides the values of the previous calls. The measured
 * energy and time values of the region are accumulated to the previous
 * measurements of this region.
 *
 * .. note::
 *    The measurements of a region must not be nested.
 *    :c:func:`EMA_region_begin` should be alternated with
 *    :c:func:`EMA_region_end`.
 *
 * @param region: The `Region` for which the measurement should started.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_region_begin(Region *region);

/**
 * This function ends a measurement for a given region. The measured energy and
 * time values of the region are accumulated to the previous measurements of
 * this region.
 *
 * .. note::
 *    Calling this function before :c:func:`EMA_region_begin` or multiple times
 *    results in undefined behaviour. :c:func:`EMA_region_end` should be
 *    alternated with :c:func:`EMA_region_begin`.
 *
 * @param region: The `Region` for which the measurement should stopped.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_region_end(Region *region);

/**
 * This function finalizes the `Region` and clears the memory.
 *
 * @param region: Pointer to a `Region` that is to be finalized.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_region_finalize(Region *region);

/* High-level region interface. */

/**
 * This function sets up a region and prepares its use in EMA if it has not
 * already been defined.
 *
 * @param region: `Region` that should be created and initialized.
 * @param idf: Region identifier or name of the `Region`.
 * @param filter: `Filter` to define `Devices` for the measurement of the
 * `Region`.
 * @param file: File name that describes where the `Region` is defined.
 * @param line: Line number that describes where the `Region` is defined.
 * @param func: Function name that describes where the `Region` is defined.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_region_define(
    Region **region,
    const char* idf,
    Filter *filter,
    const char* file,
    unsigned int line,
    const char* func
);

/**
 * This macro declares a thread-safe `Region` with a specified variable name.
 *
 * Args:
 *     region: Name of the region variable.
 *
 */
#define EMA_REGION_DECLARE(region) \
    static thread_local Region *region = NULL

/**
 * This function sets up a region and prepares its use in EMA if it has not
 * already been defined.
 *
 * .. note::
 *    To pass a `Filter` use the macro :c:macro:`EMA_REGION_DEFINE_WITH_FILTER`
 *    instead.
 *
 * Args:
 *     region: `Region` to define.
 *     idf: Identifier or name of the `Region`.
 *
 * */
#define EMA_REGION_DEFINE(region, idf) \
    EMA_REGION_DEFINE_WITH_FILTER(region, idf, NULL)

/**
 * This function sets up a region and prepares its use in EMA if it has not
 * already been defined.
 *
 * .. note::
 *    If no `Filter` is required, use the macro :c:macro:`EMA_REGION_DEFINE`
 *    instead or set `filter` to `NULL`.
 *
 * Args:
 *     region: `Region` to define.
 *     idf: Identifier or name of the `Region`.
 *     filter: A `Filter` to disable `Devices` or `Plugins`.
 */
#define EMA_REGION_DEFINE_WITH_FILTER(region, idf, filter) \
    EMA_region_define(region, idf, filter, __FILE__, __LINE__, __func__)

/**
 * This macro is an alias for :c:func:`EMA_region_begin`.
 */
#define EMA_REGION_BEGIN EMA_region_begin

/**
 * This macro is an alias for :c:func:`EMA_region_end`.
 */
#define EMA_REGION_END EMA_region_end

#endif
