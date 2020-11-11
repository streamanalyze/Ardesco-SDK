/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <ard_led.h>

//========================================================
// Program Entry Point
//========================================================
void main(void)
{
	int rc;
	printk("\nArdesco greets \"Hello World!\"\n");
	rc = led_init ();
	if (rc != 0)
	{
		printk("Failure calling led_init, rc = %d\n", rc);
	}

	while (1)
	{
		blink_led(LED_RED, 500, LED_OFF, 500, 1);
		blink_led(LED_GREEN, 500, LED_OFF, 500, 1);
		blink_led(LED_BLUE, 500, LED_OFF, 500, 1);
		k_sleep(K_MSEC(2000));
		printk ("Ping\n");
	}
}
