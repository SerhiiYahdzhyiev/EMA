#ifndef EMA_REGION_REGION_STORE_H
#define EMA_REGION_REGION_STORE_H

#include <EMA/ext/c-hashmap/map.h>
#include "region.h"

typedef struct RegionStore RegionStore;

RegionStore* EMA_region_store_init();
int EMA_region_store_set(RegionStore* store, Region* region);
size_t EMA_region_store_size(RegionStore* store);
int EMA_region_store_finalize(RegionStore* store);

typedef int (*EMA_region_iterator_cb)(Region*, void *usr);
int EMA_region_store_iterate(
    const RegionStore* store, EMA_region_iterator_cb cb, void *usr);

/* Thread-level interface. */
int EMA_thread_init();
RegionStore* EMA_thread_get_region_store();
size_t EMA_thread_get_count();
RegionStore* EMA_get_region_store(int thread_idx);
int EMA_region_stores_finalize();

#endif
