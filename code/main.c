#include <stm8s.h>
#include "tm1637.h"
#include "utils.h"

void setup(void)
{

    I2C_DeInit();
    //ADC1_DeInit();
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
    int16_t number = 0;
    bool dots = 0;
    setup();

    //GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIOB->DDR |= GPIO_PIN_5;
    GPIOB->CR1 |= GPIO_PIN_5;
    GPIOB->ODR |= GPIO_PIN_5;

    tm1637_setup();
    tm1637_set_displaying(1);

    while (1) {
        GPIOB->ODR ^= GPIO_PIN_5;
        tm1637_display(number++, dots = !dots);
        delay_ms(500);
    }
}
