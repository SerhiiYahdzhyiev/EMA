program EMA_test
    use EMA
    implicit none

    type(EMA_Region) :: region

    call EMA_init
    call EMA_region_define(region, "region1\0")

    call EMA_region_begin(region)
    call sleep(2)
    call EMA_region_end(region)

    call EMA_finalize

end program EMA_test
