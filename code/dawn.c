#include "dawn.h"
#include "rgbtape.h"

#define REDGREEN_DIFFERENCE 30
#define REDBLUE_DIFFERENCE  50
#define BRIGHTNESS_FADE 1

static volatile uint16_t red_val, green_val, blue_val;
static volatile uint8_t brightness_step_delay;

void dawn_setup(uint8_t duration)
{
    if (duration == 0)
        duration = 1;
    red_val = green_val = blue_val = 0;
    /* Для большей точности домножаем на 4, при том, что и предделитель
     * таймера_2 уменьшаем в 4 раза. Этим мы уменьшаем влияние погрешности
     * целочисленного деления. */
    brightness_step_delay = (uint16_t) duration * 60 * 4 / RGB_MAX_VALUE;
}

void dawn_stop(void)
{
    red_val = green_val = blue_val = 0;
    rgbtape_set_R(0);
    rgbtape_set_G(0);
    rgbtape_set_B(0);
    TIM2->CR1 &= ~TIM2_CR1_CEN;
}

void __dawn_irg_handler(void) __interrupt(13)
{
    static volatile uint8_t counter = 0;

    counter++;
    if (counter >= brightness_step_delay) {
        red_val += BRIGHTNESS_FADE;
        if (red_val > REDGREEN_DIFFERENCE)
            green_val += BRIGHTNESS_FADE;
        if (red_val > REDBLUE_DIFFERENCE)
            blue_val += BRIGHTNESS_FADE;

        if (red_val > RGB_MAX_VALUE)
            red_val = RGB_MAX_VALUE;
        if (green_val > RGB_MAX_VALUE)
            green_val = RGB_MAX_VALUE;
        if (blue_val > RGB_MAX_VALUE)
            blue_val = RGB_MAX_VALUE;
        rgbtape_set_R(red_val);
        rgbtape_set_G(green_val);
        rgbtape_set_B(blue_val);
        counter = 0;
    }

    TIM2->SR1 = (uint8_t) ~TIM2_IT_UPDATE;
}
