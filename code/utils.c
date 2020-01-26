#include "utils.h"

static volatile uint16_t time = 0;

void delay_ms(uint16_t us)
{
    time = 0;
    while (time < us);
}

void __delay_irq_handler(void) __interrupt(23)
{
    time++;
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}
