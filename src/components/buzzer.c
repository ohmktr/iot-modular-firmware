#include "buzzer.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define BUZZER_NODE DT_ALIAS(buzzer)
static const struct gpio_dt_spec buzzer_onboard = GPIO_DT_SPEC_GET(BUZZER_NODE, gpios);
void buzzer_init()
{
    gpio_pin_configure_dt(&buzzer_onboard, GPIO_OUTPUT);
    gpio_pin_set_dt(&buzzer_onboard, 1);
}
void buzzer_on()
{
    gpio_pin_set_dt(&buzzer_onboard, 0);
}

void buzzer_off()
{
    gpio_pin_set_dt(&buzzer_onboard, 1);
}