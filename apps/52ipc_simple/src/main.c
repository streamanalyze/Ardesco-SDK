/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>

#include <ipc_communication.h>
#include <usb_uart.h>

//========================================================
// Program Entry Point
//========================================================
void main(void)
{
	char buf[64];
    printk ("52ipc_simple started\r\n");

	// Init the comms with the 9160
	ipc_init();

	// Init the USB UART relay code.
	usb_uart_init();

	// Loop.
	while (1) 
	{
		printk ("ping52\n");
		k_sleep(K_MSEC(10 * 1000));
		//ipc_dbg_out("52Ping via 91");
		ipc_get_coproc_ver(buf, _COUNTOF(buf));
		printk ("9160 firmware version: %s\n", buf);
	}
}
