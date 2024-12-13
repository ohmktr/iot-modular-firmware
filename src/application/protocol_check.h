#ifndef CHECK_PROTO
#define CHECK_PROTO

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <uart_com.h>

extern char rx_buffer[];
bool check_device(const struct device *uart_dev, const char *device_name, int *connected_count);

#endif 