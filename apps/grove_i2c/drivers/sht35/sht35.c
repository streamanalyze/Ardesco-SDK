/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) Ericsson AB 2020, all rights reserved
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <ardesco.h>
#include <device.h>

#include <drivers/i2c.h>

#include <gpio_compat.h>    

#include <stdio.h>
#include <string.h>
#include <math.h>

#define SHT35_I2C_ADDR			0x45

// Define bytes are swapped for little endian machines. So that MSB goes out first.

#define		CMD_RESET	    	0xA230
#define		CMD_ENABLE_HEAT		0x6D30
#define		CMD_DISABLE_HEAT	0x6630
#define		CMD_GET_STATREG		0x2DF3

#define         CMD_READ_SENS_H         0x0024
#define         CMD_READ_SENS_M         0x0b24
#define         CMD_READ_SENS_L         0x1624

#define         SENSE_READ_WAIT_H       16
#define         SENSE_READ_WAIT_M       7
#define         SENSE_READ_WAIT_L       5

#define         REPEATABILITY_H         1
#define         REPEATABILITY_M         2
#define         REPEATABILITY_L         3

struct sht35_data {
    const struct device *i2c_master;
    uint8_t i2c_dev_addr;
};

struct sht35_data DevData = {0, 0};

/* DEFAULT_SET register. */
#define SHT35_DEFAULT_SET_MSK				GENMASK(7, 0)
#define SHT35_DEFAULT_SET(x)				(((x) & 0xFF) << 0)


#define BOARD_NS_GPIO_DEV_NAME "GPIO_0"
#define BOARD_NS_GROVE_SELECT_PIN 10
#define BOARD_NS_SEL_GROVEANALOG  0
#define BOARD_NS_SEL_GROVEDIGITAL 1
/*
 * ard0022B_grove_ctrl_set - Sets the Digital/Analog ctrl for the Grove Connector
 * 
 * This is an upgrade of function in board directory.
 * This function will be redundant after next SDK release.
 */
int ard0022B_grove_ctrl_set(bool fDigitalIO)
{
    int ret;
    const struct device *gpio_out_dev;
    printk ("ard0022B_grove_ctrl_set++\n");

    gpio_out_dev = device_get_binding(BOARD_NS_GPIO_DEV_NAME);
    if (!gpio_out_dev) {
            printk("Cannot find %s!", BOARD_NS_GPIO_DEV_NAME);
            return -ENODEV;
    }

    int pin = BOARD_NS_GROVE_SELECT_PIN;
    ret = gpio_pin_configure(gpio_out_dev, pin, (ARD_GPIO_OUT_ACTHI));
    if (ret) {
            printk("Error configuring pin %d!", pin);
    }
    bool sel_digital;
    if (fDigitalIO)
        sel_digital = BOARD_NS_SEL_GROVEDIGITAL;
    else
        sel_digital = BOARD_NS_SEL_GROVEANALOG;

    //ret = gpio_pin_write(gpio_out_dev, pin, sel_digital);
    ret = WRT_GPIO(gpio_out_dev, pin, sel_digital);
    if (ret) {
            printk("Error setting pin %d!", pin);
    }

    return ret;
}

static int sht35_cmd_write(struct sht35_data* dev, uint16_t cmd)
{
    return i2c_write(dev->i2c_master, (char *)&cmd, 2, dev->i2c_dev_addr);
}

static int sht35_cmd_write_readback(struct sht35_data* dev, uint16_t cmd, uint8_t *buf, 
                                    uint8_t bufsiz, uint32_t readwait)
{
    if (readwait == 0)
	    return i2c_write_read(dev->i2c_master, dev->i2c_dev_addr, &cmd, 2, buf, bufsiz);
    
    int rc = i2c_write(dev->i2c_master, (uint8_t *)&cmd, 2, dev->i2c_dev_addr);
    //      printk ("i2c_write at %x returned  %x  %d\n", addr, rc, rc);
    k_sleep(K_MSEC(readwait));

    rc = i2c_read(dev->i2c_master, buf, bufsiz, dev->i2c_dev_addr);
    //printk ("2 raw: %x %x %x %x %x %x \n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    return rc;
}

int sht35_read_statreg (struct sht35_data *dev, uint16_t *reg)
{  
    return sht35_cmd_write_readback(dev, CMD_GET_STATREG, (uint8_t *)reg, 2, 0);
}

int ReadTempHumid (struct sht35_data *dev, uint8_t repeatability, float *temp, float *humidity)
{
    uint8_t buf[6];
    float tmpf;
    int rc = 0;
    switch (repeatability)
    {
        case REPEATABILITY_H:
            rc = sht35_cmd_write_readback(dev, CMD_READ_SENS_H, (uint8_t *)buf, sizeof(buf), 
                                          SENSE_READ_WAIT_H);
            break;

        case REPEATABILITY_M:
            rc = sht35_cmd_write_readback(dev, CMD_READ_SENS_M, (uint8_t *)buf, sizeof(buf), 
                                          SENSE_READ_WAIT_M);
            break;

        case REPEATABILITY_L:
            rc = sht35_cmd_write_readback(dev, CMD_READ_SENS_L, (uint8_t *)buf, sizeof(buf), 
                                          SENSE_READ_WAIT_L);
            break;

    }   
    // Check for correct read
    if (rc != 0)
    {
        *temp = 0.0;
        *humidity = 0.0;
        return rc;
    }
    // Convert Temp
    tmpf = (float)((int32_t)buf[0] * 256 + buf[1]);
#define USE_C    
#ifdef USE_C
    *temp = ((tmpf * 175.0) / 65535.0) - 45.0;
#else
    *temp = ((tmpf * 315.0) / 65535.0) - 49.0;
#endif
    // Round to the accuracy of the sensor
    *temp = (*temp * 10.0) / 10.0;

    // Convert Humidity
    tmpf = (float)((int32_t)buf[3] * 256 + buf[4]);
    *humidity = (tmpf * 100.0) / 65535.0;

    return 0;
}

//
// External functions
//
/*
 * sht35_reset - Reset the chip.
 */ 
int sht35_reset(void)
{
    if (DevData.i2c_master == 0)
        return -ENODEV;

    return sht35_cmd_write (&DevData, CMD_RESET);

}

/*
 * sht35_GetTempHum - Returns the temperature and humidity values
 */ 
int sht35_GetTempHum(float *Temp, float *Hum)
{
    if (DevData.i2c_master == 0)
        return -ENODEV;

    return ReadTempHumid (&DevData, REPEATABILITY_H, Temp, Hum);
}

/*
 * sht35_init - Initializes the chip.
 */ 
int sht35_init (const char *i2c_name)
{
    int rc = 0;
    uint16_t stat = 0;

    rc = ard0022B_grove_ctrl_set(true);
    printk ("Grove Init rc %d\n", rc);

    DevData.i2c_master = device_get_binding(i2c_name);
    const struct device *i2c_dev = DevData.i2c_master;
    if (!DevData.i2c_master) 
    {
        printk("I2C master not found: %s", i2c_name);
        return -EINVAL;
    }
    printk("I2C master:  %x", (unsigned int)&i2c_dev);
    rc = i2c_configure (i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD));
    printk ("i2c_configure returned %d\n", rc);
	
    DevData.i2c_dev_addr = SHT35_I2C_ADDR;

    rc = sht35_read_statreg (&DevData, &stat);

    rc = sht35_reset();

    rc = sht35_read_statreg (&DevData, &stat);

    return rc;
}
