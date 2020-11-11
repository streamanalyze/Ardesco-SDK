/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <device.h>

/* Heap block space is always one of 2^(2n) for n from 3 to 7.
 * (see reference/kernel/memory/heap.html for more info)
 * Here, we target 64-byte blocks. Since we want to fit one struct uart_data
 * into each block, the block layout becomes:
 * 16 bytes: reserved by the Zephyr heap block descriptor (not in struct)
 *  4 bytes: reserved by the Zephyr FIFO (in struct)
 * 40 bytes: UART data buffer (in struct)
 *  4 bytes: length field (in struct, padded for alignment)
 */
#define UART_BUF_SIZE 40

struct uart_data {
	void *fifo_reserved;
	u8_t buffer[UART_BUF_SIZE];
	u16_t len;
};

/*
 * Function prototype for custom ISR handler
 */ 
typedef void (*uart_isr_handler) (void *user_data);

/*
 * Structure to be declared in app that defines the callback
 * function for the ISR as well as the user_data value that is
 * passed to the ISR when it is called.
 */
struct serial_isr_info {
    uart_isr_handler irq_fn;
    void *user_data;
};

/*
 * Called to initialize the serial library
 */
void serial_lib_init(void);

/*
 * Called to register a serial ISR with the library
 */
void serial_lib_register_isr (struct device *dev, struct serial_isr_info *isr_info);

/*
 * Called to initialize the USB. This function can be called 
 * multiple times but will only initialize the USB once.
 */
int common_init_usb ();
