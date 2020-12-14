/** @file
 * @brief Low Level IPC code for 9160
 *
 * Low level code for communicating with the 52840. The code uses a UART
 * with pipe code to handle the uart interrupt.
 */

/*
 * Copyright (c) 2015 Intel Corporation
 * Copyright (c) Ericsson AB 2020, all rights reserved
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Ericsson modifications.
 *   Adapted from zephyr pipe code.
 */

#include <ardesco.h>

// The app-visible defines.
#include <ipc_communication.h>
// The low level code defines.
#include <ipc_lowlevel.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ard_uart_pipe, CONFIG_UART_CONSOLE_LOG_LEVEL);

#include <drivers/uart.h>

// fallback define for non-kconfig builds.
#ifndef CONFIG_IPC_UART_DEV_NAME
#define CONFIG_IPC_UART_DEV_NAME "UART_1"
#endif // CONFIG_IPC_UART_DEV_NAME

#define UART_BUF_SIZE 40

struct uart_data {
	void *fifo_reserved;
	uint8_t buffer[UART_BUF_SIZE];
	uint16_t len;
};


typedef uint8_t *(*uart_pipe_recv_cb)(uint8_t *buf, size_t *off);

#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
static struct device *ipc91_pipe_dev = 0;
#else
const struct device *ipc91_pipe_dev = 0;
#endif

static uint8_t *recv_buf;
static size_t recv_buf_len;
static uart_pipe_recv_cb app_cb;
static size_t recv_off;

// Buffer used by serial pipe
static uint8_t pipebuf[64];

// Callback to the command processor
static coproc_recv_cb common_code_cb;

// Using two fifos to track buffer use. 
static K_FIFO_DEFINE(free_buff_fifo);
static K_FIFO_DEFINE(uart_rx_fifo);

// buffer cnt may be excessive. At least 2 needed.
#define BUF_CNT  8 // 8 to handle debug strings
struct uart_data databufs[BUF_CNT];

/*
 * ipc91_pipe_isr - handles serial interrupts
 * for IPC uart. 
 * This routine copyed from zephyr pipe.c
 */
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
static void ipc91_pipe_isr(struct device *dev)
#else
static void ipc91_pipe_isr(const struct device *dev, void *user_data)
#endif
{
	uart_irq_update(dev);

	if (uart_irq_is_pending(dev)) 
	{
		if (uart_irq_rx_ready(dev)) 
		{
			/* As per the API, the interrupt may be an edge so keep
			* reading from the FIFO until it's empty.
			*/
			for (;;) 
			{
				int avail = recv_buf_len - recv_off;
				int got;

				got = uart_fifo_read(ipc91_pipe_dev, recv_buf + recv_off, avail);
				if (got <= 0) 
				{
					break;
				}

				LOG_HEXDUMP_DBG(recv_buf + recv_off, got, "RX");

				/*
				* Call application callback with received data. Application
				* may provide new buffer or alter data offset.
				*/
				recv_off += got;
				recv_buf = app_cb(recv_buf, &recv_off);
			}
		}
	}
}

/*
 * ipc91_pipe_send - Sends data out the pipe.
 */
int ipc91_pipe_send(const uint8_t *data, int len)
{
	LOG_HEXDUMP_DBG(data, len, "TX");

	if (ipc91_pipe_dev == 0)
	{
		printk ("IPC UART Device not initialized.\n");
		return -1;
	}

	while (len--)  
	{
		uart_poll_out(ipc91_pipe_dev, *data++);
	}

	return 0;
}

/*
 * ipc91_pipe_register - Opens serial driver, registers ISR and 
 * enables interrupts.
 */
void ipc91_pipe_register(char *uart_name, uint8_t *buf, size_t len, uart_pipe_recv_cb cb)
{
	recv_buf = buf;
	recv_buf_len = len;
	app_cb = cb;

	ipc91_pipe_dev = device_get_binding(uart_name);

	if (ipc91_pipe_dev != NULL) 
	{
		uint8_t c;

		uart_irq_rx_disable(ipc91_pipe_dev);
		uart_irq_tx_disable(ipc91_pipe_dev);

		/* Drain the fifo */
		while (uart_fifo_read(ipc91_pipe_dev, &c, 1)) 
		{
			continue;
		}

		uart_irq_callback_set(ipc91_pipe_dev, ipc91_pipe_isr);

		uart_irq_rx_enable(ipc91_pipe_dev);
	}
	else
		printk ("Failed to get device binding for %s\n", uart_name);
}

// Four buffers may be excessive. At least 2 needed.
#define BUF_CNT  8 // inc'ed to handle debug strings
struct uart_data databufs[BUF_CNT];

/*
 * Using a serial pipe to field the interrupt
 * handler. The callback returns a buffer that
 * has a random number of received characters.
 * The code below collects the string until
 * an CR or 0 byte received. When detected,
 * this routine inserts the single line of
 * text in a fifo.
 * 
 * This routine could be optomized by having
 * the pipe write directly into the uart_data
 * structure buffer. Not doing that today.
 */
static uint8_t *pipe_recv_cb(uint8_t *buf, size_t *off)
{
	static struct uart_data *pcurbuf = 0;
	static uint8_t remain = 0;
	static uint8_t lastchar = 0;
	char *pnext = buf; //init not needed but calms the compiler.

//#define DBG_DMP
#ifdef DBG_DMP
	if (buf[*off-1] < ' ')
	{
		buf[*off] = '\0';
		printk ("%s\n", buf);
	}
	return buf;
#endif

	if (pcurbuf == 0)
	{
		pcurbuf = k_fifo_get(&free_buff_fifo, K_NO_WAIT);
		// See if there are no free bufs
		if (pcurbuf == 0)
		{
			printk ("out of serial buffers\n!");
			// no free buffs, pass on this notification.
			return buf;
		}
	}
	// Scan the incoming buffer for new
	// chars and check for EOL conditions.
	for (int i = lastchar; i < *off; i++)
	{
		if (pcurbuf->len < UART_BUF_SIZE-1)
		{
			if (buf[i] >= ' ')
			{
				pcurbuf->buffer[pcurbuf->len++] = buf[i];
				lastchar++;
			}
			// Just ignore
			else if (buf[i] == '\r')
			{
				continue;
			}
			else
			{
				// Check for an EOL character
				if ((buf[i] == '\n') || (buf[i] == '\0'))
				{
					// Yes, terminate string
					pcurbuf->buffer[pcurbuf->len++] = '\0';
					// 
					//printk ("inbuf: %d >%s<\n", pcurbuf->len, pcurbuf->buffer);

					// put the data buffer in the fifo
					k_fifo_put(&uart_rx_fifo, pcurbuf);

					// Record the pointer to the start of the next string
					// and the remaining chars.
					remain = *off - i - 1;
					if (remain == 0)

						*off = 0;
					pnext = &buf[i+1];

					// Get another buffer. 
					// There is the possibility that the sender sent two
					// lines faster than we could proces, so we continue.
					pcurbuf = k_fifo_get(&free_buff_fifo, K_NO_WAIT);
					// If there are no free bufs, just bail and try next time.
					if (pcurbuf == 0)
					{
						printk ("out of serial buffers 2\n!");
						break;
					}
					pcurbuf->len = 0;
				}
			}
		}
		else
		{
			// string too long. throw it out.
			printk ("Error. inbuf overflow. %d >%s<\n", pcurbuf->len, pcurbuf->buffer);
			pcurbuf->len = 0;
		}
	}
	// Move the remaining chars to the start of the pipe buffer.
	if (remain > 0)
	{
		memcpy (buf, pnext, remain);
		*off = remain;
	} 
	lastchar = *off;
	
	// This should never happen.
	if (*off >= _COUNTOF (pipebuf))
		*off = 0;
	return buf;
}
/*
 * ipc91_monitor_thread - blocks on fifo from uart_pipe. When
 * buffer avalable, sends the string to the common code for
 * processing.
 */
static void ipc91_monitor_thread(void *p1, void *p2, void *p3)
{
	while (1)
	{
		struct uart_data *buf = k_fifo_get(&uart_rx_fifo, K_FOREVER);

		/* Nothing in the FIFO, nothing to send */
		if (buf) 
		{
			// Notify app that we have an incoming command.
			if (common_code_cb)
			{
				uint8_t trm = buf->len;
				if (buf->len < UART_BUF_SIZE)
				{
					trm = buf->len+1;
				}
				else
				{
					trm = buf->len;
				}
				// Force a zero termination
				buf->buffer[trm] = '\0';
				//printk ("calling CoProc Callback\n");
				common_code_cb (buf->buffer, buf->len);
			}
		}
		else
		{
			printk ("CoProc: pulled empty buffer\n");
		}
		k_fifo_put (&free_buff_fifo, buf);
	}
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
 * ipc_lowlevel_sendstring - Sends a string to the coprocessor.
 */
int ipc_lowlevel_sendstring(char *str)
{
	//printk ("ipc_lowlevel_sendstring++\n");
	int len = strlen (str);
	return ipc91_pipe_send (str, len);
}

static K_THREAD_STACK_DEFINE(ipc91_monitor_thread_stack, /*CONFIG_BT_HCI_TX_STACK_SIZE*/ 1536);
static struct k_thread ipc91_monitor_thread_data;

/*
 * ipc_lowlevel_init - 9160-specific initialization for coprocessor communcation.
 */
int ipc_lowlevel_init (coproc_recv_cb cb)
{
	if (cb == 0)
		return -1;
	// Save the callback funciton pointer
	common_code_cb = cb;

	int i;
	// Init the bufs and put them in the free fifo
	for (i = 0; i < BUF_CNT; i++)
	{
		databufs[i].len = 0;
		k_fifo_put(&free_buff_fifo, &databufs[i]);
	}

	// Register the serial pipe.
	ipc91_pipe_register(CONFIG_IPC_UART_DEV_NAME, pipebuf, _COUNTOF(pipebuf), pipe_recv_cb);

	// Spawn the coproc monitor thread 
	k_thread_create(&ipc91_monitor_thread_data, ipc91_monitor_thread_stack,
			K_THREAD_STACK_SIZEOF(ipc91_monitor_thread_stack), ipc91_monitor_thread, NULL,
			NULL, NULL, K_PRIO_COOP(7), 0, K_NO_WAIT);
	return 0;
}
