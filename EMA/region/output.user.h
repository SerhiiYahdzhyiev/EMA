#ifndef EMA_REGION_OUTPUT_USER_H
#define EMA_REGION_OUTPUT_USER_H

#include <stdio.h>

/**
 * This function prints the measurement information of all registered `Device`
 * objects to a given file.
 *
 * @param f: Specifies a file to print to.
 *
 * @returns 0 on success or another value to indicate an error.
 */
int EMA_print_all(FILE* f);

#endif
