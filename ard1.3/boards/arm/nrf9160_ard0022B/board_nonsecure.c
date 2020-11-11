/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) Ericsson AB 2020, all rights reserved
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <init.h>

#if defined(CONFIG_BSD_LIBRARY) && defined(CONFIG_NET_SOCKETS_OFFLOAD)
#include <net/socket.h>
#endif

#include <gpio_compat.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(board_nonsecure, CONFIG_BOARD_LOG_LEVEL);

#define AT_CMD_MAX_READ_LENGTH	128
#define AT_CMD_LEN(cmd)		(sizeof (cmd) - 1)
#define AT_CMD_MAGPIO		"AT%XMAGPIO=1,1,1,9,1,698,728,"\
				"3,729,757,5,758,820,6,815,862,"\
				"2,859,895,0,896,980,1,1559,1916,"\
				"3,1917,2100,6,2101,2200"
#define AT_CMD_TRACE		"AT%XMODEMTRACE=0"
// Used to configure modem for GPS use.
#define AT_CMD_COEX0		"AT%XCOEX0=1,1,1570,1580"

static int ard0022B_magpio_configure(void)
{
#if defined(CONFIG_BSD_LIBRARY) && defined(CONFIG_NET_SOCKETS_OFFLOAD)
	int at_socket_fd;
	int buffer;
	u8_t read_buffer[AT_CMD_MAX_READ_LENGTH];

	at_socket_fd = socket(AF_LTE, 0, NPROTO_AT);
	if (at_socket_fd == -1) {
		LOG_ERR("AT socket could not be opened");
		return -EFAULT;
	}

	LOG_DBG("AT CMD: %s", log_strdup(AT_CMD_TRACE));
	buffer = send(at_socket_fd, AT_CMD_TRACE, AT_CMD_LEN(AT_CMD_TRACE), 0);
	if (buffer != AT_CMD_LEN(AT_CMD_TRACE)) {
		LOG_ERR("XMODEMTRACE command failed");
		close(at_socket_fd);
		__ASSERT_NO_MSG(false);
		return -EIO;
	}

	buffer = recv(at_socket_fd, read_buffer, AT_CMD_MAX_READ_LENGTH, 0);
	LOG_DBG("AT RESP: %s", log_strdup(read_buffer));
	if ((buffer < 2) ||
	    (memcmp("OK", read_buffer, 2 != 0))) {
		LOG_ERR("XMODEMTRACE received unexpected response");
		close(at_socket_fd);
		__ASSERT_NO_MSG(false);
		return -EIO;
	}

	LOG_DBG("AT CMD: %s", log_strdup(AT_CMD_MAGPIO));
	buffer = send(at_socket_fd, AT_CMD_MAGPIO,
		      AT_CMD_LEN(AT_CMD_MAGPIO), 0);
	if (buffer != AT_CMD_LEN(AT_CMD_MAGPIO)) {
		LOG_ERR("MAGPIO command failed");
		close(at_socket_fd);
		return -EIO;
	}

	LOG_DBG("AT CMD: %s", log_strdup(AT_CMD_COEX0));
	buffer = send(at_socket_fd, AT_CMD_COEX0,
		      AT_CMD_LEN(AT_CMD_COEX0), 0);
	if (buffer != AT_CMD_LEN(AT_CMD_COEX0)) {
		LOG_ERR("COEX0 command failed");
		close(at_socket_fd);
		return -EIO;
	}

	buffer = recv(at_socket_fd, read_buffer, AT_CMD_MAX_READ_LENGTH, 0);
	LOG_DBG("AT RESP: %s", log_strdup(read_buffer));
	if ((buffer < 2) ||
	    (memcmp("OK", read_buffer, 2 != 0))) {
		LOG_ERR("MAGPIO command failed");
		close(at_socket_fd);
		return -EIO;
	}

	LOG_DBG("MAGPIO successfully configured");

	close(at_socket_fd);
#endif
	return 0;
}

/* (db)
//#define ADP536X_I2C_DEV_NAME	DT_NORDIC_NRF_TWIM_I2C_2_LABEL
#define ADP536X_I2C_DEV_NAME	DT_LABEL(DT_NODELABEL(i2c2))
#define SPI_DEV_NAME	DT_LABEL(DT_NODELABEL(spi3))

static int ard0022B_spi_init(struct device *dev)
{
	int ret;
	struct device *gpio_out_dev;
    struct cs_gpio { char * name; int pin; int flags; } cs_gpios[DT_INST_0_NORDIC_NRF_SPIM_CS_GPIOS_COUNT] = DT_INST_0_NORDIC_NRF_SPIM_CS_GPIOS;

	gpio_out_dev = device_get_binding(SPI_DEV_NAME);
	if (!gpio_out_dev) {
		LOG_ERR("Cannot find %s!", SPI_DEV_NAME);
		return -ENODEV;
	}

	for (int i = 0; i < DT_INST_0_NORDIC_NRF_SPIM_CS_GPIOS_COUNT; i++) {
		ret = gpio_pin_configure(gpio_out_dev, cs_gpios[i].pin, (GPIO_DIR_OUT));
		if (ret) {
			LOG_ERR("Error configuring pin %d!", cs_gpios[i].pin);
		}
		ret = gpio_pin_write(gpio_out_dev, cs_gpios[i].pin, 1);
		if (ret) {
			LOG_ERR("Error setting pin %d!", cs_gpios[i].pin);
		}
	}

	return ret;
}
*/

#define BOARD_NS_SIM_SELECT_PIN 0
#define BOARD_NS_USE_ESIM 0

static int ard0022B_sim_ctrl_init(void)
{
	int ret;
	struct device *gpio_out_dev;

	gpio_out_dev = device_get_binding(BOARD_NS_GPIO_0_DEV_NAME);
	if (!gpio_out_dev) {
		LOG_ERR("Cannot find %s!", BOARD_NS_GPIO_0_DEV_NAME);
		return -ENODEV;
	}

	ret = gpio_pin_configure(gpio_out_dev, BOARD_NS_SIM_SELECT_PIN, ARD_GPIO_OUT_ACTHI);
	if (ret) {
		LOG_ERR("Error configuring pin %d!", BOARD_NS_SIM_SELECT_PIN);
	}
	bool use_esim = BOARD_NS_USE_ESIM;
	ret = WRT_GPIO(gpio_out_dev, BOARD_NS_SIM_SELECT_PIN, use_esim);
	if (ret) {
		LOG_ERR("Error setting pin %d!", BOARD_NS_SIM_SELECT_PIN);
	}

	return ret;
}

#define BOARD_NS_GROVE_SELECT_PIN 10
#define BOARD_NS_SEL_GROVEDIGITAL 1

static int ard0022B_grove_ctrl_init(void)
{
	int ret;
	struct device *gpio_out_dev;

	gpio_out_dev = device_get_binding(BOARD_NS_GPIO_0_DEV_NAME);
	if (!gpio_out_dev) {
		LOG_ERR("Cannot find %s!", BOARD_NS_GPIO_0_DEV_NAME);
		return -ENODEV;
	}

	ret = gpio_pin_configure(gpio_out_dev, BOARD_NS_GROVE_SELECT_PIN, ARD_GPIO_OUT_ACTHI);
	if (ret) {
		LOG_ERR("Error configuring pin %d!", BOARD_NS_GROVE_SELECT_PIN);
	}
	bool sel_digital = BOARD_NS_SEL_GROVEDIGITAL;
	ret = WRT_GPIO(gpio_out_dev, BOARD_NS_GROVE_SELECT_PIN, sel_digital);
	if (ret) {
		LOG_ERR("Error setting pin %d!", BOARD_NS_GROVE_SELECT_PIN);
	}

	return ret;
}
static int ard0022B_board_init(struct device *dev)
{
	int err;

	err = ard0022B_sim_ctrl_init();
	if (err) {
		LOG_ERR("ard0022B_sim_ctrl_init failed with error: %d", err);
		return err;
	}

	err = ard0022B_magpio_configure();
	if (err) {
		LOG_ERR("ard0022B_magpio_configure failed with error: %d", err);
		return err;
	}

	err = ard0022B_grove_ctrl_init();
	if (err) {
		LOG_ERR("ard0022B_grove_ctrl_init failed with error: %d", err);
		return err;
	}

	return 0;
}


SYS_INIT(ard0022B_board_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

//(db)SYS_INIT(ard0022B_spi_init, POST_KERNEL, CONFIG_SPI_INIT_PRIORITY);
