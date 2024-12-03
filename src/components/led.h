#ifndef LED_H
#define LED_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// Prototype สำหรับการควบคุม Buzzer
void led_init();                  // ฟังก์ชันสำหรับกำหนดค่า Buzzer
void led_on();                    // ฟังก์ชันเปิด Buzzer
void led_off();                   // ฟังก์ชันปิด Buzzer

#endif // LED_H