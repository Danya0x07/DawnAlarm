#ifndef _RGBSTRIP_H
#define _RGBSTRIP_H

#ifdef UNIT_TEST
#   include <stdint.h>
#   define rgbstrip_set_R(x)
#   define rgbstrip_set_G(x)
#   define rgbstrip_set_B(x)
#   define rgbstrip_is_active()
#else
#   include "halutils.h"
#   define rgbstrip_set_R(x) TIM1_SetCompare2(x)
#   define rgbstrip_set_G(x) TIM1_SetCompare1(x)
#   define rgbstrip_set_B(x) TIM1_SetCompare4(x)
#   define rgbstrip_is_active() (TIM1_GetCapture1() || TIM1_GetCapture2() || TIM1_GetCapture4())
#endif

#define RGB_MAX_VALUE 255

enum color {
    COLOR_RED = 0,
    COLOR_GREEN,
    COLOR_BLUE
};

void rgbstrip_set(enum color c, uint8_t val);
void rgbstrip_kill(void);
enum color rgbstrip_change_color(enum color c);

#endif  // _RGBSTRIP_H