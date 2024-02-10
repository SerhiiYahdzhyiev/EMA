project(EMAFortran LANGUAGES Fortran)
cmake_minimum_required(VERSION 3.17)

# EMA Fortran
add_library(EMAf SHARED)
target_sources(EMAf PRIVATE ema.f90)

# install
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ema.mod" DESTINATION "include")
install(TARGETS EMAf)
