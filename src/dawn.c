#include "dawn.h"
#include "rgbstrip.h"
#include "config.h"

#define REDGREEN_DIFFERENCE 30
#define REDBLUE_DIFFERENCE  50
#define BRIGHTNESS_FADE 1

static uint16_t red, green, blue;
static uint8_t brightness_step_delay;

void dawn_setup(uint8_t duration)
{
    if (duration == 0)
        duration = 1;
    red = green = blue = 0;
    /*
     * Для большей точности домножаем на 4, при том, что и предделитель
     * таймера_2 уменьшаем в 4 раза. Этим мы уменьшаем влияние погрешности
     * целочисленного деления.
     */
    brightness_step_delay = (uint16_t)duration * 60 * 4 / ((uint16_t)RGB_MAX_VALUE - REDBLUE_DIFFERENCE);
}

void dawn_stop(void)
{
    red = green = blue = 0;
    rgbstrip_set_R(0);
    rgbstrip_set_G(0);
    rgbstrip_set_B(0);
#if (DAWNALARM_MK == 2)
    BUZZER_GPORT->ODR &= ~BUZZER_GPIN;
#endif
    TIM2->CR1 &= ~TIM2_CR1_CEN;
}

void dawn_update(void)
{
    static uint8_t counter = 0;

    if (++counter >= brightness_step_delay) {
        red += BRIGHTNESS_FADE;
        if (red > REDGREEN_DIFFERENCE)
            green += BRIGHTNESS_FADE;
        if (red > REDBLUE_DIFFERENCE)
            blue += BRIGHTNESS_FADE;

        if (red > RGB_MAX_VALUE)
            red = RGB_MAX_VALUE;
        if (green > RGB_MAX_VALUE)
            green = RGB_MAX_VALUE;
        if (blue > RGB_MAX_VALUE) {  // синий набирает яркость последним
            blue = RGB_MAX_VALUE;
#if (DAWNALARM_MK == 2)
            BUZZER_GPORT->ODR |= BUZZER_GPIN;
#endif
        }
        rgbstrip_set_R(red);
        rgbstrip_set_G(green);
        rgbstrip_set_B(blue);
        counter = 0;
    }
}