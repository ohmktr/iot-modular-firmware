#ifndef LED_H1
#define LED_H1

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// Prototype สำหรับการควบคุม Buzzer
void led1_init();                  // ฟังก์ชันสำหรับกำหนดค่า Buzzer
void led1_on();                    // ฟังก์ชันเปิด Buzzer
void led1_off();                   // ฟังก์ชันปิด Buzzer

#endif // LED_H