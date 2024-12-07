#include "uart_com.h"
#include <string.h>
#include <stdbool.h>

/* กำหนด rx_buffer เป็นตัวแปร global */
char rx_buffer[MSG_SIZE];
static int rx_buffer_pos = 0;


void uart_send_command(const struct device *uart_dev, const char *command)
{
    for (int i = 0; command[i] != '\0'; i++)
    {
        uart_poll_out(uart_dev, command[i]);
    }
    uart_poll_out(uart_dev, '\r');
}


void uart_callback(const struct device *dev, void *user_data)
{
    uint8_t c;

    if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev))
    {
        return;
    }

    while (uart_fifo_read(dev, &c, 1) == 1)
    {
        if ((c == '\n' || c == '\r') && rx_buffer_pos > 0)
        {
            rx_buffer[rx_buffer_pos] = '\0'; // จบข้อความ
            rx_buffer_pos = 0;
        }
        else if (rx_buffer_pos < (sizeof(rx_buffer) - 1))
        {
            // เก็บข้อมูลใน buffer
            rx_buffer[rx_buffer_pos++] = c;
        }
        else
        {
            // กรณี buffer เต็ม
            printk("Buffer overflow, resetting buffer\n");
            rx_buffer_pos = 0;
        }
    }
}
