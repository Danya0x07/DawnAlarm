#include <stm8s.h>
#include "tm1637.h"
#include "utils.h"
#include "input.h"

void setup(void)
{

    I2C_DeInit();
    // ADC1 для считывания положения потенциометра
    ADC1->CR1 |= ADC1_PRESSEL_FCPU_D6;
    ADC1->CR2 |= ADC1_ALIGN_RIGHT;
    ADC1->CSR |= ADC1_CHANNEL_4;
    ADC1->CR1 |= ADC1_CR1_ADON;
    //TIM2_DeInit();

    // TIM1 для RGB-ленты
    TIM1_TimeBaseInit(20, TIM1_COUNTERMODE_UP, 255, 0);
    TIM1_OC1Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 128, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);
    TIM1_OC2Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 128, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);
    TIM1_OC4Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, 128, TIM1_OCPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET);
    TIM1_CtrlPWMOutputs(ENABLE);
    TIM1_Cmd(ENABLE);

    // TIM4 для функций задержки
    TIM4->PSCR = TIM4_PRESCALER_16;
    TIM4->ARR = 124;
    TIM4->SR1 = (uint8_t) ~TIM4_FLAG_UPDATE;
    TIM4->IER |= TIM4_IT_UPDATE;
    TIM4->CR1 |= TIM4_CR1_CEN;

    enableInterrupts();
}

int main(void)
{
    uint16_t scale = 24;
    setup();

    GPIOB->DDR |= GPIO_PIN_5;
    GPIOB->CR1 |= GPIO_PIN_5;
    GPIOB->ODR |= GPIO_PIN_5;

    input_setup();
    tm1637_setup();
    tm1637_set_displaying(1);

    while (1) {
        if (btn_pressed()) {
            uint16_t value;
            scale = scale == 24 ? 60 : 24;
            value = potentiometer_get(scale);
            tm1637_display(value, 0);
            GPIOB->ODR ^= GPIO_PIN_5;
        }
    }
}
