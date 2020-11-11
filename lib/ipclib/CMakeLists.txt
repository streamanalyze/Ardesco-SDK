#
# Copyright (c) Ericsson AB 2020, all rights reserved
#

cmake_minimum_required(VERSION 3.8.2)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.)

# Shared code between 9160 and 52840
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ipc_common.c)

# Code for 9160
target_sources_ifdef(CONFIG_SOC_NRF9160 app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ipc_91lowlevel.c)

# Code for 52840
target_sources_ifdef(CONFIG_SOC_NRF52840 app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ipc_52lowlevel.c)


