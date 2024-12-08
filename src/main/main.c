#include "uart_com.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

/* UART device configuration */
#define UART_DEVICE_NODE DT_NODELABEL(usart3)  /* Lora */
#define UART_DEVICE_NODE1 DT_NODELABEL(uart4)  /* BLE */
#define UART_DEVICE_NODE2 DT_NODELABEL(usart6) /* ESP32 */
#define VCC_NODE_LORA DT_ALIAS(vcclora)
#define PULLUP_NODE_LORA_TX DT_ALIAS(pulluploratx)
#define PULLUP_NODE_LORA_RX DT_ALIAS(pulluplorarx)

/* UART devices */
static const struct device *const lora_uart = DEVICE_DT_GET(UART_DEVICE_NODE);
static const struct device *const ble_uart = DEVICE_DT_GET(UART_DEVICE_NODE1);
static const struct device *const esp32_uart = DEVICE_DT_GET(UART_DEVICE_NODE2);

/* GPIO configuration */
static const struct gpio_dt_spec vcc_lora = GPIO_DT_SPEC_GET(VCC_NODE_LORA, gpios);
static const struct gpio_dt_spec pullup_lora_tx = GPIO_DT_SPEC_GET(PULLUP_NODE_LORA_TX, gpios);
static const struct gpio_dt_spec pullup_lora_rx = GPIO_DT_SPEC_GET(PULLUP_NODE_LORA_RX, gpios);

char last_message[MSG_SIZE] = {0};

/* RX buffer from uart_com.c */
extern char rx_buffer[];
bool check_device(const struct device *uart_dev, const char *device_name, int *connected_count)
{
    // Clear the RX buffer
    memset(rx_buffer, 0, MSG_SIZE);

    // Send AT command
    uart_send_command(uart_dev, "AT");

    // Wait for response (timeout 5000 ms, checking every 100 ms)
    int timeout = 2500;
    while (timeout > 0)
    {
        k_msleep(100); // Check every 100 ms
        timeout -= 100;

        if (strstr(rx_buffer, "+AT: OK"))
        {
            printk("|-> %s connected\n", device_name);
            (*connected_count)++;
            return true;
        }
    }

    printk("|-> %s not responding\n", device_name);
    return false;
}

void main(void)
{
    printk("|-> Starting UART Device Connection Check...\n");

    /* GPIO configuration for Lora module */
    gpio_pin_configure_dt(&vcc_lora, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&pullup_lora_tx, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&pullup_lora_rx, GPIO_OUTPUT_ACTIVE);

    /* Enable UART interrupts */
    uart_irq_rx_enable(lora_uart);
    uart_irq_rx_enable(ble_uart);
    uart_irq_rx_enable(esp32_uart);

    /* Set UART callbacks */
    uart_irq_callback_user_data_set(lora_uart, uart_callback, NULL);
    uart_irq_callback_user_data_set(ble_uart, uart_callback, NULL);
    uart_irq_callback_user_data_set(esp32_uart, uart_callback, NULL);

    /* Check devices */
    int connected_count = 0; // Counter for connected devices
    bool lora_connected = check_device(lora_uart, "Lora", &connected_count);
    bool ble_connected = check_device(ble_uart, "BLE", &connected_count);
    bool esp32_connected = check_device(esp32_uart, "ESP32", &connected_count);

    k_msleep(2500);

    /* Summary */
    printk("\n|-> Summary of Device Check:\n");
    if (connected_count > 0)
    {
        printk("|-> Total Connected Devices: %d\n", connected_count);
    }
    else
    {
        printk("|   No devices are connected.\n");
        printk("|   Please insert protocol to use.\n");
    }

    if (lora_connected)
    {
        printk("|-> Start to setup lora\n");
        uart_send_command(lora_uart, "AT+MODE=TEST");
        k_msleep(2000);
        printk("|   %s\n", rx_buffer);
        k_msleep(2000);
        uart_send_command(lora_uart, "AT+TEST=RXLRPKT");
        k_msleep(2000);
        printk("|   %s\n", rx_buffer);
    }
    k_msleep(1000);
    printk("|-> Start to Recieverฃ\n");
    uart_send_command(lora_uart, "AT+TEST=RFCFG,915.2,SF9,125,8,8,20,OFF,OFF,ON");

    while (1)
    {
        if (message_ready) // ตรวจสอบว่ามีข้อความใหม่ใน rx_buffer
        {
            if (strcmp(rx_buffer, last_message) != 0) // หากข้อความใหม่
            {
                printk("|-> Receiver is %s\n", rx_buffer);
                strncpy(last_message, rx_buffer, MSG_SIZE); // อัปเดตข้อความล่าสุด
                uart_clear_message();                       // รีเซ็ต buffer และ flag
            }
        }
        k_msleep(10); // ลดความถี่การวนลูป
    }
}
