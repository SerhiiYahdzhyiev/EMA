set(MOSQUITTO_DIR "" CACHE PATH "Path to Mosquitto installation.")

if( NOT MOSQUITTO_INCLUDE_DIR )
  find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h PATH ${MOSQUITTO_DIR}/include)
endif()

if( NOT MOSQUITTO_LIBRARY )
  find_library(MOSQUITTO_LIBRARY mosquitto PATH ${MOSQUITTO_DIR}/lib)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Mosquitto DEFAULT_MSG
  MOSQUITTO_LIBRARY MOSQUITTO_INCLUDE_DIR)
