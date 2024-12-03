#include "led.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
void led_init(){
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
}

void led_on(){
    gpio_pin_set_dt(&led, 1); // เปิด LED
}

void led_off(){
    gpio_pin_set_dt(&led, 0); // เปิด LED
}