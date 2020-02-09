#ifndef DS1307_H_INCLUDED
#define DS1307_H_INCLUDED

#include <stm8s.h>

/*
 * Формат времени для данной библиотеки: 4-хзначное число HHMM, где
 * HH - часы, MM - минуты.
 */
void ds1307_setup(uint16_t start_time);
uint16_t ds1307_get_time(void);
void ds1307_set_time(uint16_t);

#define ds1307_sq_is_1()    ((GPIOC->IDR & GPIO_PIN_3) != 0)

#endif
