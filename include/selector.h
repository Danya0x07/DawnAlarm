#ifndef _SELECTOR_H
#define _SELECTOR_H

#include "halutils.h"
#include "config.h"

void selector_set(int16_t min, int16_t max, int16_t current);
int16_t selector_get(void);

#if (DAWNALARM_MK == 1)

#define selector_irq_on()
#define selector_irq_off()

#elif (DAWNALARM_MK == 2)

#define selector_irq_off()   (ENCODER_GPORT->CR2 &= ~ENCODER_CHA_GPIN)
#define selector_irq_on()    (ENCODER_GPORT->CR2 |= ENCODER_CHA_GPIN)

INTERRUPT_HANDLER(encoder_irq, ITC_IRQ_PORTD);
#endif

#endif  // _SELECTOR_H