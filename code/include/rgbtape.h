#ifndef RGBTAPE_H_INCLUDED
#define RGBTAPE_H_INCLUDED

#include <stm8s.h>

#define rgbtape_set_R(x) TIM1_SetCompare1(x)
#define rgbtape_set_G(x) TIM1_SetCompare2(x)
#define rgbtape_set_B(x) TIM1_SetCompare4(x)

#endif
