from time import sleep

from EMA import (
    EMA_finalize,
    EMA_init,
    EMA_region_begin,
    EMA_region_define,
    EMA_region_end,
)


EMA_init()
region = EMA_region_define('region0')
EMA_region_begin(region)

print('sleep begin')
sleep(3)
print('sleep end')

EMA_region_end(region)

EMA_finalize()
