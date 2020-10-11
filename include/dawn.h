#ifndef _DAWN_H
#define _DAWN_H

#include "halutils.h"

#define dawn_start()    (TIM2->CR1 |= TIM2_CR1_CEN)
#define dawn_is_started()   ((TIM2->CR1 & TIM2_CR1_CEN) != 0)

void dawn_setup(uint8_t duration);
void dawn_stop(void);
void dawn_update(void);

#endif  // _DAWN_H