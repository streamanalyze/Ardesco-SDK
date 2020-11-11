.. Hello_52840:

Hello_52840 application
###############

The Hello_52840 application demonstrates a simple application for the 52840. It 
links to the USB_UART library to forward UART traffic from UART0 to one of two
serial USB instances created. The second serial USB instance is used for the 
console for the 52840 application. 

The application will output "52840 hello" on the second USB serial instance.

Requirements
************

* One of the following development boards:

  * |Ardesco Protptype|
  * |Ardesco Combi|
  * |Ardesco Combi Dev|
  * |Thingy91|

* usb_uart_52lib Ardesco library
* serial_52lib   Ardesco library


