#
# Copyright (c) 2018-2019 Nordic Semiconductor ASA
# Copyright (c) Ericsson AB 2020, all rights reserved
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

#
# Modified by Ericsson AB for Ardesco:
# - Based on nRF drivers
# - Changed to support Ardesco drivers
#

add_subdirectory_ifdef(CONFIG_ADP5360 adp5360)
add_subdirectory_ifdef(CONFIG_NRF_LPUART uart)