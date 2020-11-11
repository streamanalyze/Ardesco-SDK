.. 52seral_lib library:

52serial_lib
###############

The 52SERIALLIB library is included on Ardesco 52840 builds to coordinate serial ISR routines from various libraries. This is required as all 52840 serial/usb devices must point to the same isr routine.



Requirements
************

* One of the following development boards:

  * |Thingy91|
  * |Ardesco Protptype|
  * |Ardesco Combi|
  * |Ardesco Combi Dev|
  * |Ardesco Mini|


Building and running
********************
  Add the following line to the cmakelist.txt file of the 52840 Ardesco application

    add_subdirectory(${ARDESCO_ROOT}/lib/usb_uart ${CMAKE_BINARY_DIR}/lib/52serial_lib)

  In the application source
    Add the line 
        #include <52serial_lib.h>

    In the main routine of the applicaiton, insert the call
       	52serial_lib_init();

    To 'hook' a custom ISR routine, declare a struct serial_isr_info structure. 

	struct serial_isr_info {
	    uart_isr_handler irq_fn;
	    void *user_data;
	};

Set the irq_fn value to point to the custom ISR. The ISR routine should match the prototype: 

	typedef void (*uart_isr_handler) (void *user_data);

The user_data value in the struct serial_isr_info structure will be passed to the ISR when
it is called.



Dependencies
************

