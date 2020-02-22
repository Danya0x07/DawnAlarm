#ifndef DAWN_H_INCLUDED
#define DAWN_H_INCLUDED

#include <stm8s.h>

void dawn_setup(uint8_t duration);

#define dawn_start()    (TIM2->CR1 |= TIM2_CR1_CEN)
#define dawn_is_started()   ((TIM2->CR1 & TIM2_CR1_CEN) != 0)

void dawn_stop(void);

void __dawn_irg_handler(void) __interrupt(13);

#endif
