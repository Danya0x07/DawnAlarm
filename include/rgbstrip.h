#ifndef _RGBSTRIP_H
#define _RGBSTRIP_H

#include "halutils.h"

#define RGB_MAX_VALUE 255

enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

#define rgbstrip_set_R(x) TIM1_SetCompare2(x)
#define rgbstrip_set_G(x) TIM1_SetCompare1(x)
#define rgbstrip_set_B(x) TIM1_SetCompare4(x)
#define rgbstrip_is_active() (TIM1_GetCapture1() || TIM1_GetCapture2() || TIM1_GetCapture4())

void rgbstrip_set(enum color c, uint8_t val);
enum color rgbstrip_change_color(enum color c);

#endif  // _RGBSTRIP_H