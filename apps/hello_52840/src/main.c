/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>

#include <usb_uart.h>

void my_debugout_ping(struct k_work *work);

// Timer structure for sending periodic debug strings out USB
struct k_timer my_timer;

// Work structure 
K_WORK_DEFINE(my_work, my_debugout_ping);

//
// my_debugout_ping - Routine to spit out a string on one of 
// the USB uart instances.
//
void my_debugout_ping(struct k_work *work)
{
    printk ("52840 hello\r\n");
}

//
// Timer handler. Spin off real task using g_submit_work
//
void my_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&my_work);
}

//========================================================
// Program Entry Point
//========================================================
void main(void)
{
    printk ("52840 hello\r\n");

	// (db) Start the debug serial out code.
	usb_uart_init();

	// Init timer
	k_timer_init(&my_timer, my_timer_handler, NULL);

	// Start 10 sec timer, 
	k_timer_start(&my_timer, K_SECONDS(2), K_SECONDS(2));

	// Do nothing.
	while (1) 
	{
		//(db)
		k_sleep(K_MSEC(60 * 1000));
		k_yield();
                
	}
}
