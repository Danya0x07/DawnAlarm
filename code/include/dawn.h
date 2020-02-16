#ifndef DAWN_H_INCLUDED
#define DAWN_H_INCLUDED

#include <stm8s.h>

void dawn_setup(uint8_t duration);
void dawn_start(void);
bool dawn_is_started(void);
void dawn_stop(void);
void __dawn_irg_handler(void) __interrupt(13);

#endif
