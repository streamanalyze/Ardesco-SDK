/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <stdio.h>
#include <string.h>
#include <modem/lte_lc.h>

#include <sht35.h>
#include <modem/modem_info.h>

// #define TEMP_DELTA   0.1
// #define HUM_DELTA    0.1
// #define PRESS_DELTA  0.1

// //HACK:: Adjust the Thingy/Ardesco temp
// #define HACK_ADJVALC  (-1)
// #define HACK_ADJVAL   (-3)


#define BUF_SIZE 2048
char buf[BUF_SIZE];

//========================================================
// Program Entry Point
//========================================================
void main(void)
{
	int rc;
	printk("I2C over Grove example.\"\n");
	
    // Init temp device driver
    rc = sht35_init ("I2C_1");
	printk ("sht35 init returned %d\n", rc);

	// Loop forever.
	while (1)
	{
		float t, h;
		rc = sht35_GetTempHum(&t, &h);

		// printk doesn't support %f
		snprintf (buf, sizeof(buf), "Main loop. Temp: %1.1f Humidity: %1.0f\n", t, h);
		printk ("%s", buf);
		k_sleep(K_MSEC(5000));
	}
}
