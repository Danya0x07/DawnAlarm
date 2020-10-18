#ifndef _BUZZER_H
#define _BUZZER_H

#include "halutils.h"
#include "config.h"

#if (DAWNALARM_MK == 1)

#define buzzer_on()
#define buzzer_off()
#define buzzer_is_on()  (0)

#define buzzer_buzz(times, on_ms)

#elif (DAWNALARM_MK == 2)

#define buzzer_on()     (BUZZER_GPORT->ODR |= BUZZER_GPIN)
#define buzzer_off()    (BUZZER_GPORT->ODR &= ~BUZZER_GPIN)
#define buzzer_is_on()  (!!(BUZZER_GPORT->ODR & BUZZER_GPIN))

void buzzer_buzz(uint8_t times, uint8_t on_ms);
#endif  // DAWNALARM_MK

#endif  // _BUZZER_H