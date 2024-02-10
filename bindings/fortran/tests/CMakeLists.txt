project(EMAFortranTest LANGUAGES Fortran)
cmake_minimum_required(VERSION 3.17)

set(EMA_LIB "" CACHE FILEPATH "Path to EMA C library.")
set(EMA_FORTRAN_LIB "" CACHE FILEPATH "Path to EMA Fortran library.")
set(EMA_FORTRAN_INCLUDE "" CACHE PATH "Path to EMA Fortran include directory.")

# EMA Fortran
add_executable(test_emaf test_emaf.f90)
target_include_directories(test_emaf PRIVATE "${EMA_FORTRAN_INCLUDE}")
target_link_libraries(test_emaf PRIVATE "${EMA_LIB}" "${EMA_FORTRAN_LIB}")
