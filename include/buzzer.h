#ifndef _BUZZER_H
#define _BUZZER_H

#include "halutils.h"
#include "config.h"

#define buzzer_on()     (BUZZER_GPORT->ODR |= BUZZER_GPIN)
#define buzzer_off()    (BUZZER_GPORT->ODR &= ~BUZZER_GPIN)
#define buzzer_is_on()  (!!(BUZZER_GPORT->ODR & BUZZER_GPIN))

void buzzer_buzz(uint8_t times, uint8_t on_ms);

#endif  // _BUZZER_H