#include "led.h"
#include "buzzer.h"
#include "led1.h"

void main(){
    led_init();
    buzzer_init();
    led1_init();
    // buzzer_off();
    while (1)
    {
        // led_on();
        // led1_on();
        // buzzer_on();
        // k_msleep(1000);
        // led_off();
        // led1_off();
        // buzzer_off();
        // k_msleep(1000);
    }
    
}