#include <stm8s.h>


void setup(void)
{
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);
    GPIO_DeInit(GPIOD);
    TIM1_DeInit();
    TIM2_DeInit();

    // TIM4 для функций задержки
    TIM4_DeInit();
    TIM4_TimeBaseInit(TIM4_PRESCALER_16, 124);
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    TIM4_Cmd(ENABLE);
}

int main(void)
{
    while (1) {

    }
}
