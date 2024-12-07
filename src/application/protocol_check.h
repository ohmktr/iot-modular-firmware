#ifndef CHECK_PROTO
#define CHECK_PROTO

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

void check_protocol(const struct device *uart_dev);

#endif 