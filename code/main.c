#include <stm8s.h>
#include "tm1637.h"
#include "utils.h"
#include "input.h"
#include "ds1307.h"
#include "eeprom.h"

void setup(void)
{
    // ADC1 для считывания положения потенциометра
    ADC1->CR1 |= ADC1_PRESSEL_FCPU_D6;
    ADC1->CR2 |= ADC1_ALIGN_RIGHT;
    ADC1->CSR |= ADC1_CHANNEL_4;
    ADC1->CR1 |= ADC1_CR1_ADON;

    // TIM1 для RGB-ленты
    TIM1_TimeBaseInit(20, TIM1_COUNTERMODE_UP, 255, 0);
    TIM1_OC1Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);
    TIM1_OC2Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);
    TIM1_OC4Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, 0, TIM1_OCPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET);
    //TIM1_CtrlPWMOutputs(ENABLE);
    TIM1_Cmd(ENABLE);

    // TIM4 для функций задержки
    TIM4->PSCR = TIM4_PRESCALER_16;
    TIM4->ARR = 124;
    TIM4->SR1 = (uint8_t) ~TIM4_FLAG_UPDATE;
    TIM4->IER |= TIM4_IT_UPDATE;
    TIM4->CR1 |= TIM4_CR1_CEN;

    // I2C для часов реального времени и микросхемы EEPROM
    I2C_Init(I2C_MAX_STANDARD_FREQ, 0x54, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 2);
    I2C_Cmd(ENABLE);

    enableInterrupts();
}

void take_user_time(uint16_t* current_time, bool dots)
{
    uint8_t hours = *current_time / 100;
    uint8_t minutes = *current_time % 100;
    uint8_t _hours = 0, _minutes = 0;
    while (!btn_pressed()) {
        hours = potentiometer_get(24);
        if (hours != _hours) {
            tm1637_display_dec(hours * 100 + minutes, dots);
            _hours = hours;
        }
        delay_ms(10);
    }
    while (!btn_pressed()) {
        minutes = potentiometer_get(60);
        if (minutes != _minutes) {
            tm1637_display_dec(hours * 100 + minutes, dots);
            _minutes = minutes;
        }
        delay_ms(10);
    }
    *current_time = hours * 100 + minutes;
}

void take_user_dawn_duration(uint8_t* dawn_duration)
{
    uint8_t _dawn_duration = 0;
    enum tm_charset content[4] = {
        TM_d, TM_d, 0, 0
    };
    while (!btn_pressed()) {
        *dawn_duration = potentiometer_get(16);
        if (*dawn_duration != _dawn_duration) {
            content[2] = *dawn_duration / 10;
            content[3] = *dawn_duration % 10;
            tm1637_display_char(content, 0);
            _dawn_duration = *dawn_duration;
        }
        delay_ms(10);
    }
}

int main(void)
{
    static struct options opts;
    bool disp_active = 1, dots = 1, _dots = 1;
    uint16_t current_time;

    setup();
    input_setup();
    tm1637_gpio_setup();
    tm1637_set_displaying(1);
    eeprom_load(&opts);
    current_time = ds1307_get_time();

    if (btn_is_pressed()) {
        while (btn_is_pressed());
        take_user_time(&current_time, 0);
        ds1307_set_time(current_time);
    }

    while (1) {
        if (btn_pressed()) {
            if (btn_pressed_again()) {
                take_user_time(&opts.alarm_time, 1);
                take_user_dawn_duration(&opts.dawn_duration);
                eeprom_save(&opts);
            } else {
                disp_active = !disp_active;
                tm1637_set_displaying(disp_active);
            }
        }
        dots = ds1307_sq_is_1();
        if (dots != _dots) {
            tm1637_display_dec(current_time, dots);
            _dots = dots;
        }
    }
}
