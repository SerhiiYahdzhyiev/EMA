module EMA

use, intrinsic :: iso_c_binding, only: c_char, c_int, c_null_ptr, c_ptr
implicit none

! ****************************************************************************
! Fortran interface to C library of EMA.
! ****************************************************************************
interface

subroutine EMA_init_(callback) bind(c, name="EMA_init")
    import :: c_ptr
    implicit none
    type(c_ptr), value :: callback
end subroutine EMA_init_

subroutine EMA_finalize() bind(c, name="EMA_finalize")
    implicit none
end subroutine EMA_finalize

subroutine EMA_region_define_(region, idf, filter, file, line, func) bind(c, name="EMA_region_define")
    import :: c_ptr, c_char, c_int
    implicit none
    type(c_ptr), intent(out) :: region
    character(kind=c_char) :: idf(*)
    type(c_ptr), value :: filter
    character(kind=c_char) :: file(*)
    integer(kind=c_int), value :: line
    character(kind=c_char) :: func(*)
end subroutine EMA_region_define_

subroutine EMA_region_begin_(region) bind(c, name="EMA_region_begin")
    import :: c_ptr
    implicit none
    type(c_ptr), value :: region
end subroutine EMA_region_begin_

subroutine EMA_region_end_(region) bind(c, name="EMA_region_end")
    import :: c_ptr
    implicit none
    type(c_ptr), value :: region
end subroutine EMA_region_end_

subroutine EMA_region_finalize_(region) bind(c, name="EMA_region_finalize")
    import :: c_ptr
    implicit none
    type(c_ptr), value :: region
end subroutine EMA_region_finalize_

end interface

! ****************************************************************************
! Simplified Fortran wrapper to abstract from the iso_c types used internally.
! ****************************************************************************

! Region type.
type, bind(c) :: EMA_Region
    type(c_ptr) :: ptr
end type

! Wrapped EMA functions.
contains

subroutine EMA_init()
    implicit none
    call EMA_init_(c_null_ptr)
end subroutine EMA_init

subroutine EMA_region_define(region, name)
    implicit none
    type(EMA_Region), intent(out) :: region
    character(*), intent(in) :: name
    call EMA_region_define_(region%ptr, name, c_null_ptr, "\0", 0, "\0")
end subroutine EMA_region_define

subroutine EMA_region_begin(region)
    implicit none
    type(EMA_Region) :: region
    call EMA_region_begin_(region%ptr)
end subroutine EMA_region_begin

subroutine EMA_region_end(region)
    implicit none
    type(EMA_Region) :: region
    call EMA_region_end_(region%ptr)
end subroutine EMA_region_end

subroutine EMA_region_finalize(region)
    implicit none
    type(EMA_Region) :: region
    call EMA_region_finalize_(region%ptr)
end subroutine EMA_region_finalize

end module EMA
