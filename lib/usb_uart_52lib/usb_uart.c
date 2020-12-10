/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) 2020 Ericsson AB
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
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
#include <usb_uart.h>

// fallback define for non-kconfig builds.
#ifndef CONFIG_PASSTHROUGH_UART_DEV_NAME
#define CONFIG_PASSTHROUGH_UART_DEV_NAME "UART_0"
#endif //CONFIG_PASSTHROUGH_UART_DEV_NAME

// fallback define for non-kconfig builds.
#ifndef CONFIG_PASSTHROUGH_USB_DEV_NAME
#define CONFIG_PASSTHROUGH_USB_DEV_NAME "CDC_ACM_0"
#endif //CONFIG_PASSTHROUGH_USB_DEV_NAME


// Indicates USB is connected.
extern uint8_t USB_active;
extern struct k_sem power_event_sem;

static struct serial_dev {
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
	struct device *dev;
#else
	const struct device *dev;
#endif
	void *peer;
	struct k_fifo *fifo;
	struct k_sem sem;
	struct uart_data *rx;
} devs[2];

// Two fifos for the data passing from the 91 out and in to the
static K_FIFO_DEFINE(usb_0_tx_fifo);
static K_FIFO_DEFINE(uart_0_tx_fifo);

// Structures used to provide isr forwarding info
struct serial_isr_info isr_info[2];

// Structures used for usb to uart relay.
static struct serial_dev devs[2];

/* 
 * Frees data for incoming transmission on device blocked by full heap. 
 */
static int oom_free(struct serial_dev *sd)
{
	struct serial_dev *peer_sd = (struct serial_dev *)sd->peer;
	struct uart_data *buf;

	/* First, try to free from FIFO of peer device (blocked stream) */
	buf = k_fifo_get(peer_sd->fifo, K_NO_WAIT);
	if (buf) {
		k_free(buf);
		return 0;
	}

	/* Then, try FIFO of the receiving device (reverse of blocked stream) */
	buf = k_fifo_get(sd->fifo, K_NO_WAIT);
	if (buf) {
		k_free(buf);
		return 0;
	}

	/* Finally, try all of them */
	for (int i = 0; i < ARRAY_SIZE(devs); i++) {
		buf = k_fifo_get(sd->fifo, K_NO_WAIT);
		if (buf) {
			k_free(buf);
			return 0;
		}
	}

	return -1; /* Was not able to free any heap memory */
}
/*
 * uart_usb_passthrough_isr - All UARTs on the 52840
 * must share one ISR. This code was orignally designed
 * to pass data from 2 uarts to 2 instances of the USB
 * virtual serial ports. 
 * 
 * In this design we use UART1 for interprocessor 
 * communication, UART1 follows the same design but
 * instead of forwarding received data to a corresponding 
 * USB-uart, it's rx fifo is read by another thread that 
 * processes incoming strings from the 9160.
 * 
 */ 
static void uart_usb_passthrough_isr(void *user_data)
{
	struct serial_dev *sd = user_data;
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
	struct device *dev = sd->dev;
#else
	const struct device *dev = sd->dev;
#endif	
	struct serial_dev *peer_sd = (struct serial_dev *)sd->peer;

	uart_irq_update(dev);

	while (uart_irq_rx_ready(dev)) 
	{
		int data_length;

		while (!sd->rx) 
		{
			sd->rx = k_malloc(sizeof(*sd->rx));
			if (sd->rx) 
			{
				sd->rx->len = 0;
			} else 
			{
				int err = oom_free(sd);

				if (err) 
				{
					printk("Could not free memory. Rebooting.\n");
					sys_reboot(SYS_REBOOT_COLD);
				}
			}
		}

		data_length = uart_fifo_read(dev, &sd->rx->buffer[sd->rx->len],
					   UART_BUF_SIZE - sd->rx->len);
		sd->rx->len += data_length;

		if (sd->rx->len > 0) 
		{
			if ((sd->rx->len == UART_BUF_SIZE) ||
			   (sd->rx->buffer[sd->rx->len - 1] == '\n') ||
			   (sd->rx->buffer[sd->rx->len - 1] == '\r') ||
			   (sd->rx->buffer[sd->rx->len - 1] == '\0')) 
			   {
				k_fifo_put(peer_sd->fifo, sd->rx);
				k_sem_give(&peer_sd->sem);

				sd->rx = NULL;
			}
		}
	}

	if (uart_irq_tx_ready(dev)) 
	{
		struct uart_data *buf = k_fifo_get(sd->fifo, K_NO_WAIT);
		uint16_t written = 0;

		/* Nothing in the FIFO, nothing to send */
		if (!buf) 
		{
			uart_irq_tx_disable(dev);
			return;
		}

		while (buf->len > written) 
		{
			written += uart_fifo_fill(dev,
						  &buf->buffer[written],
						  buf->len - written);
		}

		while (!uart_irq_tx_complete(dev)) 
		{
			/* Wait for the last byte to get
			 * shifted out of the module
			 */
		}

		if (k_fifo_is_empty(sd->fifo)) 
		{
			uart_irq_tx_disable(dev);
		}

		k_free(buf);
	}
}

static K_THREAD_STACK_DEFINE(uart_thread_stack, /*CONFIG_BT_HCI_TX_STACK_SIZE*/ 1536);
static struct k_thread uart_thread_data;

static void usb_uart_thread(void *p1, void *p2, void *p3);

void usb_uart_init(void)
{
	// Spawn the uart thread 
	//
	k_thread_create(&uart_thread_data, uart_thread_stack,
			K_THREAD_STACK_SIZEOF(uart_thread_stack), usb_uart_thread, NULL,
			NULL, NULL, K_PRIO_COOP(7), 0, K_NO_WAIT);

	return;
}

/*
 * usb_uart_thread - Initializes the USB and UART drivers
 * used for communication. Since all UARTs on the 52840
 * must share one ISR, the code is designed to have 
 * symetric code that links the uart and USB-uart that 
 * are to relay data.
 * 
 * For interprocessor communication, UART1 follows the same
 * design but instead of forwarding received data to a 
 * corresponding USB-uart, it's rx fifo is read by another
 * thread that processes incoming strings from the 9160.
 * 
 * If the USB isn't connected, the USB and paired UART
 * are not initialized.
 */ 
static void usb_uart_thread(void *p1, void *p2, void *p3)
{
	int ret;
	struct serial_dev *usb_0_sd = &devs[0];
	struct serial_dev *uart_0_sd = &devs[1];
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
	struct device *usb_passthru_dev, *uart_passthru_dev;
#else
	const struct device *usb_passthru_dev, *uart_passthru_dev;
#endif

	// Init the 52840 common serial library.
	serial_lib_init();

	// we get the uart and usb devices even if the
	// USB is unplugged so that we can power them
	// down later in this routine.

	usb_passthru_dev = device_get_binding(CONFIG_PASSTHROUGH_USB_DEV_NAME);
	if (!usb_passthru_dev) {
		printk("%s init failed\n", CONFIG_PASSTHROUGH_USB_DEV_NAME);
	}

	uart_passthru_dev = device_get_binding(CONFIG_PASSTHROUGH_UART_DEV_NAME);
	if (!uart_passthru_dev) {
		printk("%s init failed\n", CONFIG_PASSTHROUGH_UART_DEV_NAME);
	}

	usb_0_sd->dev = usb_passthru_dev;
	usb_0_sd->fifo = &usb_0_tx_fifo;
	usb_0_sd->peer = uart_0_sd;

	uart_0_sd->dev = uart_passthru_dev;
	uart_0_sd->fifo = &uart_0_tx_fifo;
	uart_0_sd->peer = usb_0_sd;

	k_sem_init(&usb_0_sd->sem, 0, 1);
	k_sem_init(&uart_0_sd->sem, 0, 1);

	// Now init the struct to pass to the isr handler.
	isr_info[0].irq_fn = uart_usb_passthrough_isr;
	isr_info[0].user_data = usb_0_sd;

	isr_info[1].irq_fn = uart_usb_passthrough_isr;
	isr_info[1].user_data = uart_0_sd;

	if (usb_passthru_dev && uart_passthru_dev)  
	{
		serial_lib_register_isr (usb_passthru_dev, &isr_info[0]);
		serial_lib_register_isr (uart_passthru_dev, &isr_info[1]);

		ret = common_init_usb ();
		if (ret != 0) 
		{
			USB_active = 0;
			printk("Failed to enable USB\n");
		}
	}

	if (usb_passthru_dev && uart_passthru_dev)  
	{
		uart_irq_rx_enable(usb_passthru_dev);
		uart_irq_rx_enable(uart_passthru_dev);

		printk("USB <--> UART bridge is now initialized\n");

		struct k_poll_event events[3] = {
			K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
							K_POLL_MODE_NOTIFY_ONLY,
							&usb_0_sd->sem, 0),
			K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
							K_POLL_MODE_NOTIFY_ONLY,
							&uart_0_sd->sem, 0),
			K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
							K_POLL_MODE_NOTIFY_ONLY,
							&power_event_sem, 0),
		};

		while (USB_active && usb_passthru_dev && uart_passthru_dev) 
		{
			ret = k_poll(events, ARRAY_SIZE(events), K_FOREVER);
			if (ret != 0) {
				k_sleep(K_MSEC(100));
				continue;
			}
			if (events[0].state == K_POLL_TYPE_SEM_AVAILABLE) {
				events[0].state = K_POLL_STATE_NOT_READY;
				k_sem_take(&usb_0_sd->sem, K_NO_WAIT);
				uart_irq_tx_enable(usb_passthru_dev);
			} else if (events[1].state == K_POLL_TYPE_SEM_AVAILABLE) {
				events[1].state = K_POLL_STATE_NOT_READY;
				k_sem_take(&uart_0_sd->sem, K_NO_WAIT);
				uart_irq_tx_enable(uart_passthru_dev);
			} else if (events[2].state == K_POLL_TYPE_SEM_AVAILABLE) {
				events[1].state = K_POLL_STATE_NOT_READY;

				break;
			}
		}
	}
	if (usb_passthru_dev)
		uart_irq_rx_disable(usb_passthru_dev);
	if (uart_passthru_dev)
		uart_irq_rx_disable(uart_passthru_dev);

	if (usb_passthru_dev)
	{
		device_set_power_state(usb_passthru_dev, DEVICE_PM_LOW_POWER_STATE, 
		                       NULL, NULL);
	}	

	if (uart_passthru_dev)
	{
		device_set_power_state(uart_passthru_dev, DEVICE_PM_LOW_POWER_STATE, 
	                           NULL, NULL);
	}
}
