/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#ifndef ARD_LED_H__
#define ARD_LED_H__

// Include basic operating system dependencies
#include <zephyr.h>
#include <sys/types.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR == 2)
#define LED_R DT_GPIO_LEDS_LED_1_GPIOS_PIN
#define LED_B DT_GPIO_LEDS_LED_2_GPIOS_PIN
#define LED_G DT_GPIO_LEDS_LED_3_GPIOS_PIN
#endif

#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR == 3)
#define LED_R DT_N_S_leds_S_led_1_P_gpios_IDX_0_VAL_pin
#define LED_B DT_N_S_leds_S_led_2_P_gpios_IDX_0_VAL_pin
#define LED_G DT_N_S_leds_S_led_3_P_gpios_IDX_0_VAL_pin
#endif


#define LED_RED        0
#define LED_GREEN      1
#define LED_BLUE       2
#define LED_YELLOW     3
#define LED_MAGENTA    4
#define LED_CYAN       5
#define LED_WHITE      6
#define LED_OFF        7

int led_init (void);

void led_set_color(int color);

void blink_led(int on_clr, int on_time, int off_clr, int off_time, int blink_cnt);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* ARD_LED_H__ */
