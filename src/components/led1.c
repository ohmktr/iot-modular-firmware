#include "led1.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED_NODE1 DT_ALIAS(led1)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED_NODE1, gpios);
void led1_init(){
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
}

void led1_on(){
    gpio_pin_set_dt(&led1, 1); // เปิด LED
}

void led1_off(){
    gpio_pin_set_dt(&led1, 0); // เปิด LED
}