#include "uart_com.h"
#include <string.h>

/* Global buffer and status flag */
char rx_buffer[MSG_SIZE] = {0};
bool message_ready = false;
int rx_index = 0;            // ตำแหน่งสำหรับเขียนใน rx_buffer

void uart_send_command(const struct device *uart_dev, const char *command)
{
    for (int i = 0; command[i] != '\0'; i++)
    {
        uart_poll_out(uart_dev, command[i]);
    }
    uart_poll_out(uart_dev, '\r'); // ส่ง CR
}

void uart_callback(const struct device *dev, void *user_data)
{
    
    static char temp_buffer[MSG_SIZE];
    static int temp_buffer_pos = 0;
    uint8_t c;

    if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev))
    {
        return;
    }

    while (uart_fifo_read(dev, &c, 1) == 1)
    {
        if (c == '\n' || c == '\r') // เมื่อพบ newline หรือ carriage return
        {
            if (temp_buffer_pos > 0)
            {
                temp_buffer[temp_buffer_pos] = '\0'; // ใส่ null terminator
                strncpy(rx_buffer, temp_buffer, MSG_SIZE); // คัดลอกข้อความไปยัง rx_buffer
                temp_buffer_pos = 0; // รีเซ็ต temp_buffer
                message_ready = true; // ตั้ง flag
            }
        }
        else if (temp_buffer_pos < (sizeof(temp_buffer) - 1))
        {
            temp_buffer[temp_buffer_pos++] = c; // เก็บข้อมูลใน temp_buffer
        }
    }
}


void uart_clear_message(void)
{
    memset(rx_buffer, 0, MSG_SIZE); // รีเซ็ต buffer
    message_ready = false;          // เคลียร์สถานะข้อความใหม่
}
