#include "buzzer.h"

void buzzer_buzz(uint8_t times, uint8_t on_ms)
{
    while (times--) {
        buzzer_on();
        delay_ms(on_ms);
        buzzer_off();
        delay_ms(on_ms >> 1);
    }
}