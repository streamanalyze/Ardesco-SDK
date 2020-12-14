/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#ifndef ARD_USB_UART_H__
#define ARD_USB_UART_H__

// Check that we're being used in the right place.
#ifndef CONFIG_SOC_NRF52840
#error usb_uart should only be included with 52840 applications.
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the usb_uart library.
void usb_uart_init(void);

#ifdef __cplusplus
}
#endif

#endif //ARD_USB_UART_H__
