# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_IMU_otamesi_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED IMU_otamesi_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(IMU_otamesi_FOUND FALSE)
  elseif(NOT IMU_otamesi_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(IMU_otamesi_FOUND FALSE)
  endif()
  return()
endif()
set(_IMU_otamesi_CONFIG_INCLUDED TRUE)

# output package information
if(NOT IMU_otamesi_FIND_QUIETLY)
  message(STATUS "Found IMU_otamesi: 0.0.0 (${IMU_otamesi_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'IMU_otamesi' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${IMU_otamesi_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(IMU_otamesi_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${IMU_otamesi_DIR}/${_extra}")
endforeach()
