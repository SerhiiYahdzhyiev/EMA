project(EMAPython)
cmake_minimum_required(VERSION 3.17)

# Python
find_package(Python3 COMPONENTS Interpreter Development)

set(Python3_EXECUTABLE "" CACHE FILEPATH
    "Path to Python3 interpreter (keep empty for automatic search).")

if( NOT Python3_FOUND )
    message(SEND_ERROR "Python3 not found.")
endif()

# EMA
set(EMA_LIB "" CACHE FILEPATH "Path to EMA C library.")
set(EMA_INCLUDE_DIR "" CACHE PATH "Path to EMA include directory.")

if( NOT EMA_LIB )
    message(SEND_ERROR "Variable EMA_LIB not set.")
endif()

if( NOT EMA_INCLUDE_DIR )
    message(SEND_ERROR "Variable EMA_INCLUDE_DIR not set.")
endif()

# Bindings
Python3_add_library(EMA WITH_SOABI EMA.c)
target_include_directories(EMA PRIVATE "${EMA_INCLUDE_DIR}")
target_link_libraries(EMA PRIVATE "${EMA_LIB}")

# install
install(TARGETS EMA LIBRARY DESTINATION lib)
