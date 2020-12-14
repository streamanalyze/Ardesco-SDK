/*
 * Copyright (c) Ericsson AB 2020, all rights reserved
 */

#include <init.h>

#include <power/reboot.h>
#include <drivers/gpio.h>

// Set shortcuts for gpio connected to 52boot button
#define BOOT_BTN_GPIO_NUM         DT_GPIO_PIN(DT_ALIAS(btn0), gpios)
#define BOOT_BTN_GPIO_CTRLR       DT_GPIO_LABEL(DT_ALIAS(btn0), gpios)

/*
 * boot_btn_cb_fn - callback function from boot button gpio. Called
 * when the user presses the button labeled "52840 boot".
 */ 
void boot_btn_cb_fn(struct device *gpiob, 
					struct gpio_callback *cb, uint32_t pins)
{
	sys_reboot(SYS_REBOOT_COLD);
}


// gpio callback structure used by boot button callback.
static struct gpio_callback boot_btn_cb;

/*
 * config_52840_reset_btn - configure button for 52840 reset function.
 */ 
int config_52840_reset_btn ()
{
	int rc;
	struct device *gpio_dev;

	// Get GPIO driver
	gpio_dev = device_get_binding(BOOT_BTN_GPIO_CTRLR);
	if (!gpio_dev) {
		printk("Cannot find %s!\n", BOOT_BTN_GPIO_CTRLR);
		return -ENODEV;
	}

	// Configure input pin boot button.
	rc = gpio_pin_configure(gpio_dev, BOOT_BTN_GPIO_NUM, GPIO_INPUT | GPIO_PULL_UP);
	if (rc == 0) 
	{
		// disable irq
		gpio_pin_interrupt_configure(gpio_dev, BOOT_BTN_GPIO_NUM, GPIO_INT_DISABLE);
	}
	if (rc == 0) 
	{
		// Initialize callback
		gpio_init_callback(&boot_btn_cb, boot_btn_cb_fn, BIT(BOOT_BTN_GPIO_NUM));
		rc = gpio_add_callback(gpio_dev, &boot_btn_cb);
	}
	if (rc == 0) 
	{
		// Enable callback.
		rc = gpio_pin_interrupt_configure (gpio_dev, BOOT_BTN_GPIO_NUM, GPIO_INT_LEVEL_LOW);
	}
	if (rc != 0) 
	{
		printk("Unable to configure boot button for 52840!\n");
	}
    return rc;
}

static int ard0021B_board_init(struct device *dev)
{
	ARG_UNUSED(dev);

	config_52840_reset_btn ();

	return 0;
}
SYS_INIT(ard0021B_board_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
