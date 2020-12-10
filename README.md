Release Notes

Usage
	Unzip the ardesco_sdk .zip file into a directory on your hard drive.
	
	Download the Nordic Connect for Desktop tool from the Nordic website

	Use the Toolchain manager to download the Nordic SDK into the Ardesco
	directory.

	Modify the NCSDIR file in the Ardesco directory to select the proper
	NCS SDK verison. (Default is v1.3.0.)

	Open a command line build prompt using the ardcmdprompt.cmd command
	file in the ardesco directory.

	For detailed instructions, refer to "Getting Started with the Ardesco SDK"
	available on the Ardesco Teams page.

Changelog

Release 1.6
	Added board directories compatible to Nordic SDK 1.4. Set the target
	NCS version to v1.4.0, while remaining backward compatible with v1.3.

	Added sensor_lib library to provide simple API to read environment and
	accelerometer sensors.

	Added support on 52840 for LED and "boot" button in board directories 
	for Combi and Combi Dev.

	Added using_sensors example to demonstrate using the sensor library.

	Added grove_i2c example to demonstrate using I2C across Grove connector.

	Added grove_console example to demonstrate redirecting the console out the Grove connector

	Removed support for Ardesco Mini for v1.4.0

	Fixed bugs in Combi board directory that prevented successful compile.

Release 1.5
	Added missing file ard_led.h.

	Added precompiled .hex files for at_client and usb_uart_bridge samples.

	Added this readme file.

Release 1.4
	Added support for Nordic Connect SDK 1.3.x. 

	Updated build system to support multiple versions of the Nordic
	Connect SDK.

	Added Ard_LED library to simplify access to the 3-color LED.

	Added serial_52lib library to coordinate using the 52840 UARTs
	across mulitple libraries.

	Rewrote usb_uart_52lib (formerly usb_uart_lib) to support 
	serial_52lib.

	Added interprocessor communication library that uses a UART
	to communicate between the 9160 and 52840.

	Added 91ipc_simple and 52ipc_simple applications to demonstrate
	using the IPC library.

	Updated the Ardeso board directories to eliminate the predefined
	UART1 for both the 9160 and 52840 CPUs.

	Updated SDK to support both NCS 1.2.x and 1.3.x.

	Added ardcmdprompot.cmd file to open a console build window.

	Added Ardesco\Samples directory that contain the very useful 
	examples at_client (for 9160) and usb_uart_bridge (for 52840).