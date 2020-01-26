#include <stm8s.h>
#include "tm1637.h"
#include "utils.h"

void setup(void)
{

    I2C_DeInit();
    FLASH_DeInit();
    //ADC1_DeInit();
    TIM1_DeInit();
    TIM2_DeInit();

    // TIM4 для функций задержки
    TIM4_DeInit();
    TIM4_TimeBaseInit(TIM4_PRESCALER_16, 124);
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    TIM4_Cmd(ENABLE);

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
