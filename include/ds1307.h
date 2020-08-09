#ifndef _DS1307_H
#define _DS1307_H

#include "halutils.h"

/*
 * Формат времени для данной библиотеки: 4-хзначное число HHMM, где
 * HH - часы, MM - минуты.
 */
void ds1307_setup(uint16_t time);
uint16_t ds1307_get_time(void);
void ds1307_set_time(uint16_t time);

#define ds1307_sqwout_is_1()    ((GPIOC->IDR & GPIO_PIN_3) != 0)  // SQW/OUT

#endif  // _DS1307_H