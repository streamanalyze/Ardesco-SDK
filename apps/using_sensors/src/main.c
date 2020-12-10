/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <stdio.h>
#include "accel_sensor.h"
#include "env_sensor.h"

//========================================================
// Program Entry Point
//========================================================
void main(void)
{
    int rc = 0;
	void *accel_dev = 0;
	void *env_dev = 0;

	printk("\n9160 Sensor Server start.\n");

	k_sleep (K_MSEC (2000));

	// Initialize the environmental library
	env_dev = ardenv_init(&rc);
	if (env_dev == 0)
	{
		while (1)
		{
			printk ("ardenv_init: Error %d\n", rc);
			k_sleep(K_MSEC(2000));
		}
	}

	// Initialize the accel library.
	accel_dev = ardaccel_init(&rc, ACCEL_8G_DEV);
	//accel_dev = ardaccel_init(&rc, ACCEL_200G_DEV);
	if (accel_dev == 0)
	{
		while (1)
		{
			printk ("ardaccel_init: Error %d\n", rc);
			k_sleep(K_MSEC(2000));
		}
	}

    double envvals[3];
    double accelvals[3];
	char sz[128];
	while (1)
	{
		rc = ardenv_read (env_dev, &envvals, sizeof (envvals));
		rc = ardaccel_read (accel_dev, &accelvals, sizeof (accelvals));
		sprintf (sz, "%d  temp %.2f  Hum %.2f  Press %.2f     x %.2f  y %.2f  z %.2f\r\n", 
				 rc, envvals[0], envvals[1], envvals[2], accelvals[0], accelvals[1], accelvals[2]);
		printk ("%s", sz);

		k_sleep(K_MSEC(1000));
	}
}
