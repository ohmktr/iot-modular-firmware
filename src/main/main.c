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

        printk("|-> Start to scan end node (LoRa)\n");
        for (double frequency = 920.000; frequency <= 925.000; frequency += 0.600)
        {
            printk("|   Scan at frequency %f MHz\n", frequency);
            k_msleep(1000);

            char command[128];
            snprintf(command, sizeof(command), "AT+TEST=RFCFG,%.3f,SF9,125,8,8,20,OFF,OFF,ON\r\n", frequency);
            uart_send_command(lora_uart, command);
            /*Open for debug code*/
            // for (int i = 0; i < 10; i++)
            // {
            //     printk("Rx mode %s\n", rx_buffer);
            //     k_msleep(200);
            // }
            k_msleep(1000);
            uart_send_command(lora_uart, "AT+TEST=RXLRPKT\r\n");
            /*Open for debug code*/
            // for (int i = 0; i < 10; i++)
            // {
            //     printk("Rx mode %s\n", rx_buffer);
            //     k_msleep(200);
            // }
            // k_msleep(500);
            int elapsed_time = 0; // ตัวจับเวลาที่ใช้ตรวจสอบในลูป while
            while (1)
            {
                k_msleep(500);
                /*Open for debug code*/
                // printk("|   %s\n", rx_buffer);

                const char *substring = "4E4F444"; /*find node keyword*/
                if (strstr(rx_buffer, substring) != NULL)
                {
                    printk("|   Reciver %s Found!!\n", rx_buffer);
                    if (frequency_count < MAX_FREQUENCIES) // ตรวจสอบว่าไม่เกินขนาดของลิสต์
                    {
                        detected_frequencies[frequency_count++] = frequency; // บันทึกและเพิ่มตัวนับ
                    }
                    else
                    {
                        printk("Frequency list is full! Cannot save more frequencies.\n");
                    }
                    uart_send_command(lora_uart, "AT+MODE=TEST\r\n");
                    k_msleep(500);
                    break;
                }
                elapsed_time += 500;
                if (elapsed_time >= 1000) /*5 Sec*/
                {
                    printk("|   No RX message\n");
                    uart_send_command(lora_uart, "AT+MODE=TEST\r\n");
                    k_msleep(500);
                    break;
                }
                k_msleep(1000);
            }
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

    // int current_frequency_index = 0; // ตัวบอกความถี่ปัจจุบันใน detected_frequencies

    // while (1) // วนลูปไปเรื่อย ๆ
    // {
    //     if (frequency_count == 0)
    //     {
    //         printk("|-> No frequencies detected. Waiting...\n");
    //         k_msleep(5000); // หากไม่มีความถี่ให้รอ 5 วินาทีแล้วลองใหม่
    //         continue;
    //     }

    //     // ใช้ความถี่ปัจจุบัน
    //     double frequency = detected_frequencies[current_frequency_index];
    //     printk("|-> Start to Receiver at Frequency: %.3f MHz\n", frequency);

    //     // 1. ส่งคำสั่งเปลี่ยนความถี่ไปยัง LoRa
    //     char command[128];
    //     snprintf(command, sizeof(command), "AT+TEST=RFCFG,%.3f,SF9,125,8,8,20,OFF,OFF,ON\r\n", frequency);
    //     uart_send_command(lora_uart, command);
    //     k_msleep(500); // รอให้คำสั่งเปลี่ยนความถี่ทำงาน

    //     uart_clear_message();

    //     // 2. ส่งคำสั่งเพื่อรับข้อความจาก End Node
    //     uart_send_command(lora_uart, "AT+TEST=RXLRPKT\r\n");
    //     k_msleep(500); // รอ LoRa Module เข้าสู่โหมด RX
    //     uart_clear_message();

    //     // 3. รับข้อความจาก End Node
    //     bool received_message = false;
    //     while (!received_message) // วนลูปจนกว่าจะได้รับข้อความ
    //     {
    //         if (message_ready) // ตรวจสอบว่ามีข้อความใหม่ใน rx_buffer
    //         {
    //             if (strncmp(rx_buffer, "+TEST: RX", 9) == 0) // ตรวจสอบว่าเป็นข้อความจาก End Node
    //             {
    //                 printk("|   Receiver is %s\n", rx_buffer); // แสดงข้อความจาก End Node
    //                 received_message = true;                   // ข้อความได้รับเรียบร้อย
    //                 uart_clear_message();
    //             }
    //             else
    //             {
    //                 // หากเป็นข้อความอื่น เช่น Response ของ LoRa Module
    //                 printk("|   Receiver is %s\n", rx_buffer);
    //                 uart_clear_message();
    //             }
    //         }

    //         k_msleep(10); // ลดความถี่การวนลูป
    //     }

    //     // เปลี่ยนไปความถี่ถัดไปทันทีที่ได้รับข้อความ
    //     current_frequency_index = (current_frequency_index + 1) % frequency_count; // หมุนกลับไปความถี่แรกเมื่อจบ
    //     printk("|-----------------------------------------|\n");
    // }

    int current_frequency_index = 0; // ตัวบอกความถี่ปัจจุบันใน detected_frequencies

    while (1) // วนลูปไปเรื่อย ๆ
    {
        if (frequency_count == 0)
        {
            printk("|-> No frequencies detected. Waiting...\n");
            k_msleep(5000); // หากไม่มีความถี่ให้รอ 5 วินาทีแล้วลองใหม่
            continue;
        }

        // ใช้ความถี่ปัจจุบัน
        double frequency = detected_frequencies[current_frequency_index];
        printk("|-> Start to Receiver at Frequency: %.3f MHz\n", frequency);

        // 1. ส่งคำสั่งเปลี่ยนความถี่ไปยัง LoRa
        char command[128];
        snprintf(command, sizeof(command), "AT+TEST=RFCFG,%.3f,SF9,125,8,8,20,OFF,OFF,ON\r\n", frequency);
        uart_send_command(lora_uart, command);
        k_msleep(500); // รอให้คำสั่งเปลี่ยนความถี่ทำงาน

        uart_clear_message(); // เคลียร์ buffer ก่อนเข้าสู่โหมด RX

        // 2. ส่งคำสั่งเพื่อรับข้อความจาก End Node
        uart_send_command(lora_uart, "AT+TEST=RXLRPKT\r\n");
        k_msleep(500); // รอ LoRa Module เข้าสู่โหมด RX
        uart_clear_message();

        // 3. รับข้อความจาก End Node
        bool received_len_info = false;
        bool received_rx_info = false;

        int len = 0;
        int rssi = 0;
        int snr = 0;
        char rx_data[50] = {0};

        while (!received_len_info || !received_rx_info) // วนลูปจนกว่าจะได้รับทั้งสองข้อความ
        {
            if (message_ready) // ตรวจสอบว่ามีข้อความใหม่ใน rx_buffer
            {
                if (strncmp(rx_buffer, "+TEST: LEN", 10) == 0) // ตรวจสอบข้อความ +TEST: LEN
                {
                    sscanf(rx_buffer, "+TEST: LEN:%d, RSSI:%d, SNR:%d", &len, &rssi, &snr);
                    printk("|   LEN: %d, RSSI: %d, SNR: %d\n", len, rssi, snr);
                    // printk("|   Receiver is %s\n", rx_buffer);
                    received_len_info = true;
                    uart_clear_message();
                }
                else if (strncmp(rx_buffer, "+TEST: RX", 9) == 0) // ตรวจสอบข้อความ +TEST: RX
                {
                    sscanf(rx_buffer, "+TEST: RX \"%[^\"]\"", rx_data);
                    printf("|   RX Data: %s\n", rx_data);
                    // printk("|   Receiver is %s\n", rx_buffer);
                    received_rx_info = true;
                    uart_clear_message();
                }
                else
                {
                    // หากเป็นข้อความอื่น
                    printk("|   Ignored Response: %s\n", rx_buffer);
                    uart_clear_message();
                }
            }

            k_msleep(10); // ลดความถี่การวนลูป
        }

        // แสดงเส้นแบ่งเมื่อรับข้อมูลครบในความถี่ปัจจุบัน
        printk("|----------------------------------------------------|\n");

        // เปลี่ยนไปความถี่ถัดไปทันทีเมื่อรับข้อมูลครบ
        current_frequency_index = (current_frequency_index + 1) % frequency_count; // หมุนกลับไปความถี่แรกเมื่อจบ
    }
}