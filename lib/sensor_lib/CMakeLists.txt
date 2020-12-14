#
# Copyright (c) Ericsson AB 2020, all rights reserved
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

zephyr_include_directories(.)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/env.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/accel.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sensor_common.c)
