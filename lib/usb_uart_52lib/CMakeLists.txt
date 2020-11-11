#
# Copyright (c) Ericsson AB 2020, all rights reserved
#

cmake_minimum_required(VERSION 3.8.2)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.)

# Code only for 52840
target_sources_ifdef(CONFIG_SOC_NRF52840 app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/usb_uart.c)
