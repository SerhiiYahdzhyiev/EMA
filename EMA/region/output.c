#include "output.h"
#include "region.h"
#include "region_store.h"

int EMA_print_header(FILE* f)
{
    int ret = fprintf(
        f,
        "thread,region_idf,file,line,function,visits,"
        "device_name,device_type,energy,time\n"
    );
    return ret >= 0 ? 0 : 1;
}

int EMA_print_region(const Region* region, int thread_idx, FILE* f)
{
    for(int i = 0; i < region->measurements.size; ++i)
    {
        const Measurement *measurement = region->measurements.array + i;
        int ret = fprintf(
            f, "%d,%s,%s,%d,%s,%llu,%s,%s,%llu,%llu\n",
            thread_idx,
            region->idf,
            region->file,
            region->line,
            region->function,
            region->visits,
            measurement->device->name,
            measurement->device->type,
            measurement->energy_result,
            measurement->time_result
        );
        if( ret < 0 )
            return 1;
    }
    return 0;
}

typedef struct
{
    int thread_idx;
    FILE* f;
} PrintIterator;

int _EMA_print_region_iterator(Region* region, void *usr)
{
    PrintIterator *it = usr;
    return EMA_print_region(region, it->thread_idx, it->f);
}

int EMA_print_region_store(const RegionStore* store, int thread_idx, FILE* f)
{
    PrintIterator it = { .thread_idx = thread_idx, .f = f };
    return EMA_region_store_iterate(store, _EMA_print_region_iterator, &it);
}

int EMA_print_all(FILE* f)
{
    EMA_print_header(f);
    for(int i = 0; i < EMA_thread_get_count(); ++i)
    {
        const RegionStore* store = EMA_get_region_store(i);
        int ret = EMA_print_region_store(store, i, f);
        if( ret != 0 )
            return ret;
    }
    return 0;
}
