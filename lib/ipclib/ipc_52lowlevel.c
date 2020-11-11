/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 * Copyright (c) 2020 Ericsson AB
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
/*
 * Derived from usb_uart_bridge.c 
 * Extensively modified to use one USB channel for 
 * passthrough and the other for 52840 console out.
 * 
 * One UART is assigned for passthrough. The other
 * is used for interprocess communcation with 9160.
 */ 

#include <ardesco.h>

#include <device.h>
#include <drivers/uart.h>
#include <hal/nrf_power.h>
#include <power/reboot.h>
#include <usb/usb_device.h>

#include <serial_52lib.h>

// The app-visible defines.
#include <ipc_communication.h>
// The low level code defines.
#include <ipc_lowlevel.h>

// fallback define for non-kconfig builds.
#ifndef CONFIG_IPC_UART_DEV_NAME
#define CONFIG_IPC_UART_DEV_NAME "UART_1"
#endif //CONFIG_IPC_UART_DEV_NAME

void disable_uart(int uartnum);
int send_debug_out (char *dbgstr);

static struct serial_isr_info ipc_isr_info[2];

// Flag to signal coproc comm shutdown.
bool fStopComms = false;
extern int wake_timer_is_set;

// These are used for inter-mpu comms
static K_FIFO_DEFINE(usb_console_tx_fifo);
static K_FIFO_DEFINE(usb_console_rx_fifo);
static K_FIFO_DEFINE(uart_ipc_rx_fifo);

static struct ipc_serial_dev {
	struct device *dev;
	struct k_fifo *rx_fifo;
	struct k_fifo *tx_fifo;
	struct uart_data *rx;
} ipcdevs[2];

struct device *uart_ipc_dev = 0;

// These serial dev structures must be visible outside
// the init code so that the isr can know which uart
// caused the interrupt.
struct ipc_serial_dev *uart_ipc_sd;
struct ipc_serial_dev *usb_console_sd;

// Callback to common code.
static coproc_recv_cb app_cb;
static int in_call;

int usb_isr = 0;
int usb_rx_isr = 0;
int usb_tx_isr = 0;

static u8_t buffer[UART_BUF_SIZE];

/*
 * ipc_interrupt_handler - All UARTs on the 52840
 * must share one ISR so we are called indirectly
 * from the commin serial52 lib isr.
 * 
 */ 
static void ipc_interrupt_handler(void *user_data)
{
	struct ipc_serial_dev *sd = user_data;
	struct device *dev = sd->dev;

	uart_irq_update(dev);

	while (uart_irq_rx_ready(dev)) 
	{
		int data_length;

		while (!sd->rx) 
		{
			sd->rx = k_malloc(sizeof(*sd->rx));
			if (sd->rx) {
				sd->rx->len = 0;
			} else 
			{
				printk("Could not free memory. discarding data.\n");
				//sys_reboot(SYS_REBOOT_COLD);
				data_length = uart_fifo_read(dev, buffer, UART_BUF_SIZE);
				break;						
			}
		}
		if (sd->rx)
		{
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
					k_fifo_put(sd->rx_fifo, sd->rx);
					sd->rx = NULL;
				}
			}
		}
	}

	if (uart_irq_tx_ready(dev)) 
	{
		struct uart_data *buf = k_fifo_get(sd->tx_fifo, K_NO_WAIT);
		u16_t written = 0;

		/* Nothing in the FIFO, nothing to send */
		if (!buf) {
			uart_irq_tx_disable(dev);
			return;
		}

		while (buf->len > written) {
			written += uart_fifo_fill(dev,
						  &buf->buffer[written],
						  buf->len - written);
		}

		while (!uart_irq_tx_complete(dev)) {
			/* Wait for the last byte to get
			 * shifted out of the module
			 */
		}

		if (k_fifo_is_empty(sd->tx_fifo)) {
			uart_irq_tx_disable(dev);
		}
		k_free(buf);
	}
}

/*
 * ipc_lowlevel_sendstring - Sends a string to the other processor.
 */
int ipc_lowlevel_sendstring (char *str)
{
	//printk ("coproc_sendstring++\n");
	if (uart_ipc_dev == 0)
		return -1;
	while (!fStopComms && (*str != '\0'))  
	{
		uart_poll_out(uart_ipc_dev, *str++);
	}
	return 0;
}

/*
 * ipc_lowlevel_console_out - Sends a string out console
 */
void ipc_lowlevel_console_out (char *str)
{
	printk ("%s", str);
	return;
}

/*
 * ipc52_monitor_thread - Blocks on the receive fifo for uart1
 * and sends the line of text to the command dispatcher.
 */
static void ipc52_monitor_thread(void *p1, void *p2, void *p3)
{
	//printk ("ipc52_monitor_thread++\n");
	in_call = 1;

	uart_ipc_sd = &ipcdevs[0];

	// Using UART for 52840 - 9160 Comms
	uart_ipc_dev = device_get_binding(CONFIG_IPC_UART_DEV_NAME);
	if (uart_ipc_dev) 
	{
		uart_ipc_sd->dev = uart_ipc_dev;
		uart_ipc_sd->rx = 0;
		uart_ipc_sd->rx_fifo = &uart_ipc_rx_fifo;

		// Now init the struct to pass to the isr handler.
		ipc_isr_info[1].irq_fn = ipc_interrupt_handler;
		ipc_isr_info[1].user_data = uart_ipc_sd;
		serial_lib_register_isr (uart_ipc_dev, &ipc_isr_info[1]);
		uart_irq_rx_enable(uart_ipc_dev);

		while (!fStopComms)
		{
			in_call--;
			struct uart_data *buf = k_fifo_get(&uart_ipc_rx_fifo, K_FOREVER);
			in_call++;

			/* Nothing in the FIFO, nothing to send */
			if (!buf) {
				continue;
			}
			if (buf->len == 0x1234)
				break;

			// Notify app that we have an incoming command.
			if (app_cb)
			{
				// Force a zero termination. If the buff is
				// totally full, zero out the last byte. Otherwise,
				// the string terminated 1 char earlier.
				u8_t trm = buf->len;
				if (buf->len == UART_BUF_SIZE)
				{
					trm = buf->len;
				}
				else
				{
					trm = buf->len-1;
				}
				buf->buffer[trm] = '\0';
				//printk ("calling CoProc Callback\n");
				app_cb (buf->buffer, buf->len);
			}
			k_free(buf);
		}
		uart_irq_rx_disable(uart_ipc_dev);

		device_set_power_state(uart_ipc_dev, DEVICE_PM_LOW_POWER_STATE, 
	                           NULL, NULL);
	}
	in_call = 12345;

	k_sleep(K_MSEC(2000));
//	printk ("ipc thd exit\n");
}

static K_THREAD_STACK_DEFINE(ipc52_monitor_thread_stack, /*CONFIG_BT_HCI_TX_STACK_SIZE*/ 1536);
static struct k_thread ipc52_monitor_thread_data;

/*
 * ipc_lowlevel_shutdown - shuts down the low level interprocessor communication layer.
 */
int ipc_lowlevel_shutdown (void)
{
	//printk("ipc_lowlevel_shutdown++\r\n");
	int rc = 0;
	static struct uart_data dummy;
	memset (&dummy, 0, sizeof (dummy));
	dummy.len = 0x1234;

	fStopComms = true;
	if (in_call == 0)
	{
		// Trigger the recv thread.
		k_fifo_put(&uart_ipc_rx_fifo, &dummy);
	}
	//printk("ipc_lowlevel_shutdown--\r\n");
	return rc;
}
/*
 * coproc_cpu_init - Initializes low level interprocessor communication layer.
 */
int ipc_lowlevel_init (coproc_recv_cb cb)
{
	//printk ("ipc_lowlevel_init++\n");

	fStopComms = false;

	// Init hte serial library of no one else has.
	serial_lib_init();

	// Save the callback func
	app_cb = cb;

	// Spawn the ipc monitor thread 
	k_thread_create(&ipc52_monitor_thread_data, ipc52_monitor_thread_stack,
			K_THREAD_STACK_SIZEOF(ipc52_monitor_thread_stack), ipc52_monitor_thread, NULL,
			NULL, NULL, K_PRIO_COOP(7), 0, K_NO_WAIT);
	// Let it run
	k_sleep(K_MSEC(10));
	return 0;
}

#define UART52_0_PWR_REG   (*(volatile uint32_t *)0x40002FFC)
#define UART52_1_PWR_REG   (*(volatile uint32_t *)0x40028000)
/*
 * disable_uart - Code from Nordic to truly shut down
 * a nrf52 uart.
 */
void disable_uart(int uartnum)
{
	if (uartnum == 0)
	{
		NRF_UARTE0->EVENTS_RXTO = 0;
		NRF_UARTE0->TASKS_STOPRX = 1;
		while(NRF_UARTE0->EVENTS_RXTO == 0){}
		NRF_UARTE0->EVENTS_RXTO = 0;
		NRF_UARTE0->ENABLE = 0;

		//Workaround. Power cycle UART0
		UART52_0_PWR_REG = 0;
		UART52_0_PWR_REG;
		UART52_0_PWR_REG = 1;
	}
	else if (uartnum == 1)
	{
		NRF_UARTE1->EVENTS_RXTO = 0;
		NRF_UARTE1->TASKS_STOPRX = 1;
		//(db)while(NRF_UARTE1->EVENTS_RXTO == 0){}
		k_sleep(K_MSEC(10));
		NRF_UARTE1->EVENTS_RXTO = 0;
		NRF_UARTE1->ENABLE = 0;

		//Workaround. Power cycle UART1
		UART52_1_PWR_REG = 0;
		UART52_1_PWR_REG;
		UART52_1_PWR_REG = 1;
	}
}
