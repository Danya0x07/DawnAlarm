#include "selector.h"
#include "config.h"

#if (DAWNALARM_MK == 1)

#define ADC_ADJUST_SHIFT    (-15)

static int16_t bd_min, bd_max, current;

static void potentiometer_update(void)
{
    uint16_t adc_value;

    ADC1->CR1 |= ADC1_CR1_ADON;
    while (!(ADC1->CSR & ADC1_FLAG_EOC));
    adc_value = ADC1->DRL;
    adc_value |= ADC1->DRH << 8;

    if (adc_value > 500 && bd_max < 200)  // Небольшая корректировка значения.
        adc_value += ADC_ADJUST_SHIFT;
    current = (uint32_t)adc_value * (bd_max - bd_min) / 1023 + bd_min;

    if (current > bd_max)
        current = bd_max;
    if (current < bd_min)
        current = bd_min;
}
#elif (DAWNALARM_MK == 2)

static volatile int16_t bd_min, bd_max, current;

INTERRUPT_HANDLER(encoder_irq, ITC_IRQ_PORTD)
{
    selector_irq_off();

    delay_ms(2);
    if (!(ENCODER_GPORT->IDR & ENCODER_CHA_GPIN))
        goto out;

    if (ENCODER_GPORT->IDR & ENCODER_CHB_GPIN) {
        current++;
    } else {
        current--;
    }
    if (current > bd_max)
        current = bd_max;
    if (current < bd_min)
        current = bd_min;

out:
    selector_irq_on();
}
#endif  // DAWNALARM_MK

void selector_set(int16_t _min, int16_t _max, int16_t _current)
{
    selector_irq_off();
    bd_min = _min;
    bd_max = _max;
    current = _current;
    selector_irq_on();
}

int16_t selector_get(void)
{
#if (DAWNALARM_MK == 1)
    potentiometer_update();
#endif
    return current;
}