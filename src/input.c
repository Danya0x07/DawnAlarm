#include "input.h"
#include "utils.h"

#define DEBOUNCE_DURATION_MS    5
#define DOUBLECLICK_SCAN_CYCLES 370
#define ADC_ADJUST_SHIFT    (-15)

enum btn_pull {PULLDOWN, PULLUP};

struct button {
    GPIO_TypeDef* reg;
    uint8_t pin;
    enum btn_pull pull;
    bool last_state;
} btn;

void input_setup(void)
{
    btn.reg = GPIOA;
    btn.pin = GPIO_PIN_1;
    btn.pull = PULLDOWN;
}

bool btn_pressed(void)
{
    bool pressed = FALSE;
    bool current_state = btn_is_pressed();
    if (btn.last_state != current_state) {
        delay_ms(DEBOUNCE_DURATION_MS);
        current_state = btn_is_pressed();
    }

    if (!btn.last_state && current_state) pressed = !(bool)btn.pull;
    else if (btn.last_state && !current_state) pressed = (bool)btn.pull;
    btn.last_state = current_state;
    return pressed;
}

bool btn_pressed_again(void)
{
    uint16_t i;
    for (i = 0; i < DOUBLECLICK_SCAN_CYCLES; i++) {
        delay_ms(1);
        if (btn_pressed()) {
            return TRUE;
        }
    }
    return FALSE;
}

bool btn_is_pressed(void)
{
    return (btn.reg->IDR & btn.pin) != btn.pull;
}

uint16_t potentiometer_get(uint16_t scale)
{
    uint16_t adc_value;
    uint32_t result;
    ADC1->CR1 |= ADC1_CR1_ADON;
    while (!(ADC1->CSR & ADC1_FLAG_EOC));
    adc_value = ADC1->DRL;
    adc_value |= ADC1->DRH << 8;
    if (adc_value > 500 && scale < 200)  // Небольшая корректировка значения.
        adc_value += ADC_ADJUST_SHIFT;
    result = (uint32_t)adc_value * scale / 1023;
    if (result > scale)
        result = scale;
    if (scale == 256 && result == scale)  // Откровенный костыль, но лучше так.
        result = 255;
    return result;
}
