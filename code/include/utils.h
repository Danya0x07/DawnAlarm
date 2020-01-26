#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stm8s.h>

void delay_us(uint16_t);
void delay_ms(uint16_t);

INTERRUPT_HANDLER(__delay_irq_handler, 23);

#endif
