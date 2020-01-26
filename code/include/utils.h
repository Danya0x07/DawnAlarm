#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stm8s.h>

void delay_ms(uint16_t);
void __delay_irq_handler(void) __interrupt(23);

#endif
