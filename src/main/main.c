#include "uart_com.h"
#include "protocol_check.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <led.h>
#include <led1.h>

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
#define MAX_FREQUENCIES 100
double detected_frequencies[MAX_FREQUENCIES]; // ลิสต์สำหรับเก็บความถี่
int frequency_count = 0;                      // ตัวนับจำนวนความถี่ที่ถูกบันทึก

void main(void)
{
    printk("|-> Starting UART Device Connection Check...\n");
    led_init();
    led1_init();

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

    k_msleep(2500);
    /* Check devices */
    int connected_count = 0; // Counter for connected devices
    bool lora_connected = check_device(lora_uart, "Lora", &connected_count);
    bool ble_connected = check_device(ble_uart, "BLE", &connected_count);
    bool esp32_connected = check_device(esp32_uart, "ESP32", &connected_count);

    k_msleep(1000);

    /* Summary */
    printk("\n|-> Summary of Device Check:\n");
    if (connected_count > 0)
    {
        printk("|   Total Connected Devices: %d\n", connected_count);
    }
    else
    {
        printk("|   No devices are connected.\n");
        printk("|   Please insert protocol to use.\n");
    }

    k_msleep(1000);
    if (lora_connected)
    {
        /*Setup lora*/
        printk("|-> Start to setup lora\n");
        uart_send_command(lora_uart, "AT+MODE=TEST\r\n");
        k_msleep(500);
        printk("|   %s\n", rx_buffer);
        k_msleep(500);
        uart_send_command(lora_uart, "AT+TEST=RXLRPKT\r\n");
        k_msleep(500);
        printk("|   %s\n", rx_buffer);

        k_msleep(2000);
        printk("|-> Start to scan end node (LoRa)\n");

        for (double frequency = 915.200; frequency <= 918.200; frequency += 0.600)
        {
            // ส่งคำสั่ง AT พร้อมความถี่ปัจจุบัน
            char command[128];
            snprintf(command, sizeof(command), "AT+TEST=RFCFG,%.3f,SF9,125,8,8,20,OFF,OFF,ON\r\n", frequency);
            uart_send_command(lora_uart, command);
            printk("|-> Scan at frequency: %.3f\n", frequency);
            k_msleep(500);

            int elapsed_time = 0; // ตัวจับเวลาที่ใช้ตรวจสอบในลูป while

            while (1)
            {
                if (message_ready) // ตรวจสอบว่ามีข้อความใหม่ใน rx_buffer
                {
                    if (strstr(rx_buffer, "+TEST: RX") != NULL) // หากพบข้อความ "+TEST: RX"
                    {
                        led_on();
                        printk("|   Found!\n");
                        if (frequency_count < MAX_FREQUENCIES) // ตรวจสอบว่าไม่เกินขนาดของลิสต์
                        {
                            detected_frequencies[frequency_count++] = frequency; // บันทึกและเพิ่มตัวนับ
                        }
                        else
                        {
                            printk("Frequency list is full! Cannot save more frequencies.\n");
                        }
                        memset(rx_buffer, 0, sizeof(rx_buffer)); // เคลียร์ทุกบิตในบัฟเฟอร์
                        break;                                   // ออกจากลูป while และเปลี่ยนความถี่
                    }
                    memset(rx_buffer, 0, sizeof(rx_buffer)); // เคลียร์ทุกบิตในบัฟเฟอร์
                }

                k_msleep(10);       // ลดความถี่การวนลูป
                elapsed_time += 10; // เพิ่มเวลาที่ใช้ไปในลูป

                if (elapsed_time >= 1000) // หากครบ 3 วินาที
                {
                    printk("|       No RX message\n");
                    break; // ออกจากลูป while และไปที่ความถี่ถัดไป
                }
            }
            led_off();
            memset(rx_buffer, 0, sizeof(rx_buffer)); // เคลียร์ทุกบิตในบัฟเฟอร์
            k_msleep(500);                           // หน่วงเวลา 500 มิลลิวินาทีก่อนเปลี่ยนความถี่
            memset(rx_buffer, 0, sizeof(rx_buffer)); // เคลียร์ทุกบิตในบัฟเฟอร์
        }
        k_msleep(1000);
        if (frequency_count > 0)
        {
            printk("|-> Detected Frequencies: ");
            for (int i = 0; i < frequency_count; i++)
            {
                if (i > 0) // ใส่เครื่องหมาย "," หากไม่ใช่ค่าความถี่ตัวแรก
                {
                    printk(",");
                }
                printk("'%.3f'", detected_frequencies[i]);
            }
            printk("\n"); // ขึ้นบรรทัดใหม่หลังจากแสดงผลทั้งหมด
        }
        else
        {
            printk("|-> No Frequencies Detected.\n");
        }
    }

    else if (esp32_connected)
    {
        /*Setup esp32*/
        printk("Hello test2");
        return;
    }

    else if (ble_connected)
    {
        /*BLE*/
        printk("Hello test3");
        return;
    }
}
