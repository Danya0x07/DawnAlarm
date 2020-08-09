#ifndef _SELECTOR_H
#define _SELECTOR_H

#include "halutils.h"

void selector_set(int16_t min, int16_t max, int16_t current);
int16_t selector_get(void);

#if (DAWNALARM_MK == 2)
INTERRUPT_HANDLER(encoder_irq, ITC_IRQ_PORTD);
#endif

#endif  // _SELECTOR_H