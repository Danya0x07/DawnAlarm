#ifndef _RGBSTRIP_H
#define _RGBSTRIP_H

#ifdef UNIT_TEST
#   include <stdint.h>
#   include <stdbool.h>
#   define rgbstrip_set(a, b)
#else
#   include "halutils.h"
#   define rgbstrip_set_R(x) _TIM1_SetCompare2(x)
#   define rgbstrip_set_G(x) _TIM1_SetCompare1(x)
#   define rgbstrip_set_B(x) _TIM1_SetCompare4(x)
#endif

#define RGB_MAX_VALUE 255

enum color {
    COLOR_RED = 0,
    COLOR_GREEN,
    COLOR_BLUE
};

void rgbstrip_set(enum color c, uint8_t val);
void rgbstrip_kill(void);
bool rgbstrip_is_active(void);

#endif  // _RGBSTRIP_H