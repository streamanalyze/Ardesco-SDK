/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) Ericsson AB 2020, all rights reserved
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/*
 * Modified by Ericsson AB for Ardesco:
 * - Changed to support Ardesco battery monitoring
 */
/**@file
 *
 * @defgroup battery_charge Battery Charge
 * @brief  Module that governs the battery charge of the system.
 * @{
 */

#ifndef BATTERY_CHARGE_H__
#define BATTERY_CHARGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init the battery charge monitor module.
 *
 */
void battery_monitor_init(int poll_interval);

/**
 * @brief Read the current battery charge.
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int battery_monitor_read(u8_t *buf);

/**
 * @brief Read the current battery charge state.
 *
 * @return 0 if the operation was successful, otherwise a (negative) error code.
 */
int battery_charge_status_read(u8_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* BATTERY_CHARGE_H__ */
