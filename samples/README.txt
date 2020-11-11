These samples are copied from the Nordic build tree from the following directories:
nrf\samples\usb\nrf9160\at_client and nrf\samples\usb\usb_uart_bridge

The CMAKELIST files have been modified to build under the Ardesco Tree.

Both of these samples are very useful for Ardesco developers. The at_client
example expose the AT commands of the modem out the debug console of the application.
When used with the usb_uart_bridge sample for the 52840, the Nordic desktop tool 
LTE_Monitor can be used to experiment with AT commands.

The usb_uart_bridge application is handy to reflect two serial ports from the 9160
to the USB. The difference between this 52840 application and the usb_uart_52lib library
in the Ardesco SDK is that this application makes the ardesco "look" like a Thingy91 
to the connected PC enabling the Nordic Tools to recognize it.

Each application has precompiled .hex files for the Ardesco Prototype and the 
Ardesco Combi. The Ardeco Combi Dev uses the same .hex file as the Combi.