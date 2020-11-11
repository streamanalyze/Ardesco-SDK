#
# Copyright (c) Ericsson AB 2020, all rights reserved
#

cmake_minimum_required(VERSION 3.8.2)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/serial_52lib.c)
