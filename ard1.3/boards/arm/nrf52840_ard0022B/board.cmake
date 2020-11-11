# Copyright (c) 2019 Nordic Semiconductor ASA.
# Copyright (c) Ericsson AB 2020, all rights reserved
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic

board_runner_args(nrfjprog "--nrf-family=NRF52")
board_runner_args(jlink "--device=nrf52" "--speed=4000")
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
