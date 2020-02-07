#ifndef TM1637_H_INCLUDED
#define TM1637_H_INCLUDED

#include <stm8s.h>

/* Яркость сегментов [0; 7]. */
#define TM_BRIGHTNESSS  7

void tm1637_setup(void);
/* Отображает десятичное число number [-999; 9999] на дисплее,
 * dots == 1: отображать двоеточие, 0: не отображать. */
void tm1637_display(int16_t number, bool dots);
/* Переключает состояние дисплея,
 * displaying == 1: сегменты светятся, 0: не светятся. */
void tm1637_set_displaying(bool displaying);

#endif
