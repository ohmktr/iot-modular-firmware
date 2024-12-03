#ifndef BUZZER_H
#define BUZZER_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// Prototype สำหรับการควบคุม Buzzer
void buzzer_init();                  // ฟังก์ชันสำหรับกำหนดค่า Buzzer
void buzzer_on();                    // ฟังก์ชันเปิด Buzzer
void buzzer_off();                   // ฟังก์ชันปิด Buzzer

#endif 