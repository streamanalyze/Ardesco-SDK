/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#ifndef ARD_GPIOCOMPAT_H__
#define ARD_GPIOCOMPAT_H__

// make sure gpio.h is included.
#include <drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Gpio calls changed from 1.2 to 1.3.
// Declare macros to hide the problem.
//
#define BOARD_NS_GPIO_0_DEV_NAME "GPIO_0"
#define BOARD_NS_GPIO_1_DEV_NAME "GPIO_1"


//Breaking changes from 1.2 to 1.3
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR == 2)
#define ARD_GPIO_OUT_ACTLOW  GPIO_DIR_OUT
#define WRT_GPIO(a,b,c)      gpio_pin_write(a,b,c);
#endif
#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR == 3)
#define ARD_GPIO_OUT_ACTLOW  (GPIO_OUTPUT | GPIO_ACTIVE_LOW)
#define ARD_GPIO_OUT_ACTHI   (GPIO_OUTPUT | GPIO_ACTIVE_HIGH)
#define WRT_GPIO(a,b,c)      gpio_pin_set(a,b,c!=0)
#endif


/*
 * Common lines between the 9160 and 52840.
 * 
 * There are 8 lines between the 9160 and 52840. The
 * first 4 are used as a HW flow controlled UART.
 * The remaining 4 can be used as another UART, if
 * defined in an overlay file for both sides, or
 * as descrete GPIOs
 * 
 * TODO: Define these from the DTS compiled result.
 * 
 * NOTE: This pin assignments are good for the Combi
 * and Combi Dev as well as the Prototype and Mini.
 * 
 */
#ifdef CONFIG_SOC_NRF9160
#define MCU_4_PIN        22
#define MCU_5_PIN        23
#define MCU_6_PIN        24
#define MCU_7_PIN        25
#endif // CONFIG_SOC_NRF9160

#ifdef CONFIG_SOC_NRF52840
#define MCU_4_PIN       32
#define MCU_5_PIN       25
#define MCU_6_PIN       19
#define MCU_7_PIN       22
#endif // CONFIG_SOC_NRF52840

/* 
 * Used by 9160 to select digital vs analog on Grove data pins
 * on the Combi Dev.
 * This is defined for the Combi as well to maintain sw compat.
 */ 
#if defined CONFIG_BOARD_NRF9160_ARD0021BNS || defined CONFIG_BOARD_NRF9160_ARD0022BNS
#define BOARD_NS_GROVE_SELECT_PIN 10
#define BOARD_NS_SEL_GROVEDIGITAL 1
#endif //CONFIG_BOARD_NRF9160_ARD0021BNS || CONFIG_BOARD_NRF9160_ARD0022BNS


/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* ARD_GPIOCOMPAT_H__ */
