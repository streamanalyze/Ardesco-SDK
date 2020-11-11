#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
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

project(USBUartBridge)
zephyr_compile_definitions(PROJECT_NAME=${PROJECT_NAME})

target_sources(app PRIVATE src/main.c)
zephyr_include_directories(src)
