#include "dawn.h"
#include "rgbtape.h"

#define MAX_COLOR_VALUE 100
#define REDGREEN_DIFFERENCE 20
#define REDBLUE_DIFFERENCE  30

static volatile uint16_t red_val, green_val, blue_val;
static volatile uint8_t fade;

void dawn_setup(uint8_t duration)
{
    red_val = green_val = blue_val = 0;
    fade = MAX_COLOR_VALUE / duration;
}

void dawn_start(void)
{
    TIM2->CR1 |= TIM2_CR1_CEN;
}

bool dawn_is_started(void)
{
    return (TIM2->CR1 & TIM2_CR1_CEN) != 0;
}

void dawn_stop(void)
{
    rgbtape_set_R(0);
    rgbtape_set_G(0);
    rgbtape_set_B(0);
    TIM2->CR1 &= ~TIM2_CR1_CEN;
}

void __dawn_irg_handler(void) __interrupt(13)
{
    static volatile uint8_t counter = 0;
    counter++;
    if (counter >= 60) {
        red_val += fade;
        if (red_val > REDGREEN_DIFFERENCE)
            green_val += fade;
        if (red_val > REDBLUE_DIFFERENCE)
            blue_val += fade;

        if (red_val > MAX_COLOR_VALUE)
            red_val = MAX_COLOR_VALUE;
        if (green_val > MAX_COLOR_VALUE)
            green_val = MAX_COLOR_VALUE;
        if (blue_val > MAX_COLOR_VALUE)
            blue_val = MAX_COLOR_VALUE;
        rgbtape_set_R(red_val);
        rgbtape_set_G(green_val);
        rgbtape_set_B(blue_val);
        counter = 0;
    }

    TIM2->SR1 = (uint8_t ) ~TIM2_IT_UPDATE;
}
