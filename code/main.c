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

    enableInterrupts();
    // I2C для часов реального времени и микросхемы EEPROM

    I2C_Init(I2C_MAX_STANDARD_FREQ, 0x54, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 2);
    I2C_Cmd(ENABLE);
}

int main(void)
{
    static struct options opts;

    setup();
    input_setup();
    tm1637_setup();
    tm1637_set_displaying(1);
    //ds1307_setup(1234);
    tm1637_display(0, 1);
    while (1) {
        if (btn_pressed()) {
            eeprom_load(&opts);
            //opts.alarm_time = 8888;
            tm1637_display(opts.alarm_time, 0);
            //eeprom_save(&opts);
            tm1637_display(opts.alarm_time, 1);
        }
    }
}
