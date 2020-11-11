.. 52ipc_simple:

52ipc_simple application
###############

The 52ipc_simple demonstrates a simple communication between the 52840 and 9160.  
The 9160 must be programmed with 91ipc_simple to work. The application uses
the IPC library to send text out the 9160's debug console.

The application also relays UART0 out the USB to relay the 9160 debug console
out the USB.

It links to the following libraries

	ipclib		- supports communication between the 52840 and 9160
	usb_uart_52lib	- supports relay from uart to USB serial. 
	serial_52lib	- supports sharing of 52840 serial interrupts across
			  different libraries.

 to forward UART traffic from UART0 to one of two
serial USB instances created. The second serial USB instance is used for the 
console for the 52840 application. 

The application will output "52840 hello" on the second USB serial instance.

Requirements
************

* One of the following development boards:

  * |Ardesco Protptype|
  * |Ardesco Combi|
  * |Ardesco Combi Dev|
  * |Ardesco Mini|
  * |Thingy91|

* USB_UART Ardesco library


