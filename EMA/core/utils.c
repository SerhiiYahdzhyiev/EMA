#include <stdlib.h>

void* realloc_s(void* ptr, size_t size)
{
    if( size > 0 )
        return realloc(ptr, size);

    free(ptr);
    return NULL;
}
