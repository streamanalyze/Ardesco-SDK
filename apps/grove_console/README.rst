.. Grove_console:

grove_console application
###############

The grove_console application demonstrates redirection of the console out the 
Grove connector configured as UART1. It will output "ping" to the console. 

This example is identical to the hello_world example with the exception of the 
nrf9160_ard0022Bns.overlay file. This file maps UART1 TX and RX pins to the
two data pins on the Grove connector. This file also redirects the console from
UART0 to UART1.

The Grove connector should be wired to a TTL level UART. The most common example
would be a FIDI board such as the Sparkfun FIDI Basic. The FIDI board is then
connected via USB to a PC where the console can be opened. The terminal program
for the console should be set to 115K N81.

Using a standard Grove Breakout cable, make the following connections:

Black wire to Ground
Yellow wire to Rx
Red wire to Tx

The advantage of mapping the console to the Grove connector is that there is 
no dependency on the USB being connected or the 52840 relaying the console data 
from the 9160 to the USB.

Because of the dependency of the Grove connector, this sample will only run on
the Combi Dev.


Requirements
************

* One of the following development boards:

  * |Ardesco Combi Dev|


