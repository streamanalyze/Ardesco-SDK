/*
 * Copyright (c) 2020 Ericsson AB
 *
 */
/*
 * Derived from usb_uart_bridge.c
 */ 

#include <ardesco.h>

#include <device.h>
#include <drivers/uart.h>
#include <hal/nrf_power.h>
#include <power/reboot.h>
#include <usb/usb_device.h>

#include <serial_52lib.h>

// Indicates USB is connected.
u8_t USB_active = 1;

#define POWER_THREAD_STACKSIZE		CONFIG_IDLE_STACK_SIZE
#define POWER_THREAD_PRIORITY		K_LOWEST_APPLICATION_THREAD_PRIO

/*
 * uart_interrupt_handler - All UARTs on the 52840
 * must share one ISR. This code receives all isr 
 * callbacks and forwards them to the approperate
 * routine using the saved function pointer provided
 * when the callback was initialized.
 * 
 */ 
static void uart_interrupt_handler(void *user_data)
{
	struct serial_isr_info *isr_info = user_data;

	isr_info->irq_fn (isr_info->user_data);
	return;
}


static K_THREAD_STACK_DEFINE(power_thread_stack, POWER_THREAD_STACKSIZE);
static struct k_thread power_thread_data;

struct k_sem power_event_sem;

/*
 * power_thread - This code clears the USB_active flag when a
 * usb connection is no longer detected
 */
static void power_thread(void *p1, void *p2, void *p3)
{
	while (USB_active) 
	{
		if (!nrf_power_usbregstatus_vbusdet_get(NRF_POWER)) 
		{
			USB_active = 0;
			// This allows signalling of the disconnect event.
			k_sem_give(&power_event_sem);
			return;
		}
		k_sleep(K_MSEC(100));
	}
}
static int usb_enabled = 0;
/*
 * common_init_usb - coordinates multiple libraries that may
 * 'enable' the usb library.
 *
 */ 
int common_init_usb ()
{
	int rc;
	if (usb_enabled > 0)
		return 0;
	rc = usb_enable(NULL);
	if (rc == 0)
		usb_enabled++;
	return rc;
}
static u8_t seral_lib_initialized = 0;
/*
 * serial_lib_init - Initializes the library.
 *
 */ 
void serial_lib_init(void)
{
	// allow multiple calls safely.
	if (seral_lib_initialized++ > 0)
		return;

	k_sem_init(&power_event_sem, 0, 1);

	USB_active = 1;

	// Start power thread to monitor USB.
	k_thread_create(&power_thread_data, power_thread_stack,
			K_THREAD_STACK_SIZEOF(power_thread_stack), power_thread, NULL,
			NULL, NULL, POWER_THREAD_PRIORITY, 0, K_NO_WAIT);
	return;
}

/*
 * serial_lib_register_isr - register an isr
 *
 */ 
void serial_lib_register_isr (struct device *dev, struct serial_isr_info *isr_info)
{
	uart_irq_callback_user_data_set(dev, uart_interrupt_handler, isr_info);
}

