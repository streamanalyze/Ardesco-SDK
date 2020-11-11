#
# Copyright (c) Ericsson AB 2020, all rights reserved
#

cmake_minimum_required(VERSION 3.8.2)

set(APP_VERSION "0.10")

# Include Ardesco SDK 
set(AR $ENV{ARDESCO_ROOT})
if(DEFINED AR)
   include($ENV{ARDESCO_ROOT}/ardesco.cmake)
else()
   # Assume we are building from the apps folder in Ardesco tree
   include(${CMAKE_CURRENT_SOURCE_DIR}/../../ardesco.cmake)
endif()

project(NONE)

target_sources(app PRIVATE src/main.c)

# Add lib for 91-52 communication 
add_subdirectory(${ARDESCO_LIB_DIR}/ipclib ${CMAKE_BINARY_DIR}/lib/ipclib)

# Add lib for serial on 52840
add_subdirectory(${ARDESCO_LIB_DIR}/serial_52lib ${CMAKE_BINARY_DIR}/lib/serial_52lib)

# Add lib for usb support on 52840
add_subdirectory(${ARDESCO_LIB_DIR}/usb_uart_52lib ${CMAKE_BINARY_DIR}/lib/usb_uart_52lib)


