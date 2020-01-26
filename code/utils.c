#include "utils.h"

static volatile uint16_t time = 0;

INTERRUPT_HANDLER(__delay_irq_handler, 23)
{
    time++;
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}

void delay_us(uint16_t us)
{
    time = 0;
    while (time < us);
}

void delay_ms(uint16_t ms)
{
    uint16_t i;
    for (i = 0; i < ms; i++)
        delay_us(1000);
}
