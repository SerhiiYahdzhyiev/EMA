#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include <EMA/user.h>
#include "region_store.h"

typedef struct RegionStore
{
    hashmap* hashmap;
} RegionStore;

char* EMA_get_region_key(Region* region)
{
    char *key = NULL;
    int ret = asprintf(
        &key, "%s:%d(%s:%s)",
        region->file, region->line, region->function, region->idf);
    if( ret == -1 )
        return NULL;
    return key;
}

RegionStore* EMA_region_store_init()
{
    RegionStore *store = malloc(sizeof(RegionStore));
    if( !store )
        return NULL;

    store->hashmap = hashmap_create();
    if( !store->hashmap )
        return NULL;

    return store;
}

int EMA_region_store_set(RegionStore* store, Region* region)
{
    const char *key = EMA_get_region_key(region);
    if( !key )
        return 1;
    hashmap_set(store->hashmap, key, strlen(key), (uintptr_t) region);
    return 0;
}

size_t EMA_region_store_size(RegionStore* store)
{
    return hashmap_size(store->hashmap);
}

static void _EMA_region_finalize_iterator(
    void* key, unsigned long ksize, uintptr_t value, void *usr)
{
    Region* region = (Region*) value;
    int *err = usr;
    int ret = EMA_region_finalize(region);
    if( ret != 0 )
        *err = ret;
    free(key);
}

int EMA_region_store_finalize(RegionStore* store)
{
    int err = 0;
    hashmap_iterate(store->hashmap, _EMA_region_finalize_iterator, &err);
    hashmap_free(store->hashmap);
    return err;
}

typedef struct
{
    EMA_region_iterator_cb cb;
    void *usr;
    int err;
} RegionIterator;

static void _EMA_region_iterator(
    void* key, unsigned long ksize, uintptr_t value, void *usr)
{
    Region* region = (Region*) value;
    RegionIterator *it = usr;
    int ret = it->cb(region, it->usr);
    if( ret != 0 )
        it->err = ret;
}

int EMA_region_store_iterate(
    const RegionStore* store, EMA_region_iterator_cb cb, void *usr)
{
    RegionIterator it = { .cb = cb, .usr = usr, .err = 0 };
    hashmap_iterate(store->hashmap, _EMA_region_iterator, &it);
    return it.err;
}

/* Thread-level interface. */
#define THREAD_LIMIT 1024

static atomic_int EMA_thread_count = 0;
static thread_local int EMA_local_thread_idx = -1;
static RegionStore* EMA_region_store[THREAD_LIMIT];

int EMA_thread_init()
{
    if( EMA_local_thread_idx >= 0 )
        return 0;

    EMA_local_thread_idx = atomic_fetch_add(&EMA_thread_count, 1);
    if( EMA_local_thread_idx >= THREAD_LIMIT )
        return 1;

    EMA_region_store[EMA_local_thread_idx] = EMA_region_store_init();

    return 0;
}

RegionStore* EMA_thread_get_region_store()
{
    if( EMA_local_thread_idx >= 0 )
        return EMA_region_store[EMA_local_thread_idx];
    return NULL;
}

size_t EMA_thread_get_count()
{
    return EMA_thread_count;
}

RegionStore* EMA_get_region_store(int thread_idx)
{
    return EMA_region_store[thread_idx];
}

int EMA_region_stores_finalize()
{
    for(size_t i = 0; i < EMA_thread_count; ++i)
        EMA_region_store_finalize(EMA_region_store[i]);
    return 0;
}
