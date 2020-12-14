#
# Copyright (c) Ericsson AB 2020, all rights reserved
#

cmake_minimum_required(VERSION 3.8.2)

# Include Ardesco SDK 
set(AR $ENV{ARDESCO_ROOT})
if(DEFINED AR)
   include($ENV{ARDESCO_ROOT}/ardesco.cmake)
else()
   # Assume we are building from the apps folder in Ardesco tree
   include(${CMAKE_CURRENT_SOURCE_DIR}/../../ardesco.cmake)
endif()

project(grove_i2c)
zephyr_compile_definitions(PROJECT_NAME=${PROJECT_NAME})

if(APP_VERSION)
  zephyr_compile_definitions(APP_VERSION=${APP_VERSION})
endif()

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)
# NORDIC SDK APP END

# Add i2c temp sensor
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/drivers/sht35 ${CMAKE_BINARY_DIR}/drivers/sht35)

