#ifndef UART_COMM_H
#define UART_COMM_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

/* ประกาศตัวแปร rx_buffer ให้เป็น global */
#define MSG_SIZE 256
extern char rx_buffer[MSG_SIZE]; 
extern bool message_ready;


// ฟังก์ชันสำหรับส่งคำสั่ง AT Command
void uart_send_command(const struct device *uart_dev, const char *command);

// ฟังก์ชันสำหรับอ่านข้อมูลตอบกลับ
void uart_callback(const struct device *dev, void *user_data);


#endif // UART_COMM_H
