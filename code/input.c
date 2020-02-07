#include "input.h"
#include "utils.h"

// Подтяжка кнопки
typedef enum {PULLDOWN, PULLUP = !PULLDOWN} ButtonMode;

struct button {
    GPIO_TypeDef* reg;
    uint8_t pin;
    ButtonMode mode;
    bool last_state;
} btn;

void input_setup(void)
{
    btn.reg = GPIOA;
    btn.pin = GPIO_PIN_1;
    btn.mode = PULLDOWN;
}

bool btn_pressed(void)
{
    bool pressed = FALSE;
    bool current_state = btn_is_pressed();
    if (btn.last_state != current_state) {
        delay_ms(5);
        current_state = btn_is_pressed();
    } else {
        return FALSE;
    }

    if (!btn.last_state && current_state) pressed = !(bool)btn.mode;
    else if (btn.last_state && !current_state) pressed = (bool)btn.mode;
    btn.last_state = current_state;
    return pressed;
}

bool btn_pressed_again(void)
{
    uint8_t i;
    for (i = 0; i < 200; i++) {
        delay_ms(1);
        if (btn_pressed()) {
            return TRUE;
        }
    }
    return FALSE;
}

bool btn_is_pressed(void)
{
    return (btn.reg->IDR & btn.pin) != btn.mode;
}

uint16_t potentiometer_get(uint16_t scale)
{
    uint16_t adc_value;
    ADC1->CR1 |= ADC1_CR1_ADON;
    while (!(ADC1->CSR & ADC1_FLAG_EOC));
    adc_value = ADC1->DRL;
    adc_value |= ADC1->DRH << 8;
    if (adc_value > 500)  // небольшая корректировка значения, чтобы не вылезало
        adc_value -= 15;  // за рамки (scale).
    return  (uint32_t)adc_value * scale / 1023;
}
