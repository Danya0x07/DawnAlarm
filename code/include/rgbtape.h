#ifndef RGBTAPE_H_INCLUDED
#define RGBTAPE_H_INCLUDED

#include <stm8s.h>

#define RGB_MAX_VALUE 255

enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

#define rgbtape_set_R(x) TIM1_SetCompare2(x)
#define rgbtape_set_G(x) TIM1_SetCompare1(x)
#define rgbtape_set_B(x) TIM1_SetCompare4(x)
#define rgbtape_is_active() (TIM1_GetCapture1() || TIM1_GetCapture2() || TIM1_GetCapture4())

void rgbtape_set(enum color, uint8_t);

#endif
