/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <ardesco.h>
#include <gpio_compat.h>
#include <ard_led.h>

/*
 * Led Definitions and Strucures
 */
typedef struct 
{
    int red;
    int green;
    int blue;
} led_state;

static led_state led_seq[] = 
{
    {1,0,0},    /* Red     */
    {0,0,1},    /* Green   */
    {0,1,0},    /* Blue    */
    {1,0,1},    /* Yellow  */
    {1,1,0},    /* Magenta */
    {0,1,1},    /* Cyan    */
    {1,1,1},    /* White   */
    {0,0,0},    /* OFF     */
 
};

#if (NRF_VERSION_MAJOR == 1) && (NRF_VERSION_MINOR < 4)
struct device *gpio_out_dev = 0;
#else
const struct device *gpio_out_dev = 0;
#endif

/*
 * Function: blink_led
 *
 * Description:            
 * Blinks the LED one color, then another for the specified times.
 * 
 */
void blink_led(int on_clr, int on_time, int off_clr, int off_time, int blink_cnt)
{
    if (gpio_out_dev == 0)
    {
        return;
    }
	// LED indication: green at start and red at stop
	for (int i = 0; i < blink_cnt; i++) 
    {
        led_set_color (on_clr);
		k_sleep(K_MSEC(on_time));
        led_set_color (off_clr);
		k_sleep(K_MSEC(off_time));
	}
}

/*
 * Function: led_set_color
 *
 * Description:            
 * Set led color 
 * 
 */
void led_set_color(int color)
{
    if (gpio_out_dev == 0)
    {
        return;
    }
    WRT_GPIO(gpio_out_dev, LED_RED, led_seq[color].red);
    WRT_GPIO(gpio_out_dev, LED_GREEN, led_seq[color].green);
    WRT_GPIO(gpio_out_dev, LED_BLUE, led_seq[color].blue);
}

/*
 * Function: led_init
 * 
 * Description:                
 * Initialize the LEDs  library
 */
int led_init(void)
{
    gpio_out_dev = device_get_binding(BOARD_NS_GPIO_0_DEV_NAME);
    if (!gpio_out_dev) 
    {
        return -1;
    }
    gpio_pin_configure(gpio_out_dev, LED_RED, ARD_GPIO_OUT_ACTLOW);
    gpio_pin_configure(gpio_out_dev, LED_GREEN, ARD_GPIO_OUT_ACTLOW);
    gpio_pin_configure(gpio_out_dev, LED_BLUE, ARD_GPIO_OUT_ACTLOW);
    return 0;
}
