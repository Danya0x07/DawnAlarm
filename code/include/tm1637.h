#ifndef TM1637_H_INCLUDED
#define TM1637_H_INCLUDED

#include <stm8s.h>

#define TM_BRIGHTNESSS  7
void tm1637_setup(void);
void tm1637_display(int16_t number, bool dots);
void tm1637_set_displaying(bool displaying);

#endif
