#include "protocol_check.h"
bool check_device(const struct device *uart_dev, const char *device_name, int *connected_count)
{
    // Clear the RX buffer
    memset(rx_buffer, 0, MSG_SIZE);
    uart_send_command(uart_dev, "AT");

    // Wait for response (timeout 5000 ms, checking every 100 ms)
    int timeout = 2500;
    while (timeout > 0)
    {
        k_msleep(100); // Check every 100 ms
        timeout -= 100;

        if (strstr(rx_buffer, "+AT: OK"))
        {
            printk("|   %s connected\n", device_name);
            (*connected_count)++;
            return true;
        }
    }

    printk("|   %s not responding\n", device_name);
    return false;
}
