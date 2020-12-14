/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) Ericsson AB 2020, all rights reserved
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#ifndef SHT35_H_
#define SHT35_H_

/**@file sht35.h
 *
 * @brief Driver for SHT35.
 * @defgroup sht35 Driver for SHT35
 * @{
 */

#include <zephyr.h>

#include "env_sensors.h"

/**
 * @brief Initialize SHT35.
 *
 * @param[in] dev_name Pointer to the device name.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int sht35_init(const char *dev_name);


/**
 * @brief Read the STATUS1 register.
 *
 * @param[out] buf The read value of the STATUS1 register.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int sht35_status_read(uint8_t *buf);


/**
 * @brief Reset the device to its default values.
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int sht35_reset(void);


/**
 * @brief Read temp and humidity values
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int sht35_GetTempHum(float *Temp, float *Hum);


int ard0022B_grove_ctrl_init_z(bool fDigitalIO);

#endif /* SHT35_H_ */
