/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <ipc_communication.h>

//========================================================
// Program Entry Point
//========================================================
void main(void)
{
	char buf[64];
	printk("91ipc_simple started.\n");

	// Initialize the IPC library
	ipc_init ();

	// Loop
	while (1)
	{
		k_sleep(K_MSEC(5000));
		printk ("Ping91\n");

		ipc_get_coproc_ver(buf, _COUNTOF(buf));
		printk ("52840 firmware version: %s\n", buf);
	}
}
