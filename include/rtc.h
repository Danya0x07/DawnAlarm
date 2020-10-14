#ifndef _RTC_H
#define _RTC_H

#include "halutils.h"
#include "config.h"

#define rtc_irq_off()   (RTC_SQW_OUT_GPORT->CR2 &= ~RTC_SQW_OUT_GPIN)
#define rtc_irq_on()    (RTC_SQW_OUT_GPORT->CR2 |= RTC_SQW_OUT_GPIN)
#define rtc_sqwout_is_high()    (!!(RTC_SQW_OUT_GPORT->IDR & RTC_SQW_OUT_GPIN))

#define RTC_NVRAM_SAVE  0
#define RTC_NVRAM_LOAD  1

/*
 * Формат времени для данной библиотеки: 4-хзначное число HHMM, где
 * HH - часы, MM - минуты.
 */
void rtc_setup(uint16_t time);
bool rtc_is_running(void);
void rtc_set_time(uint16_t time);
uint16_t rtc_get_time(void);
void rtc_access_nvram(uint8_t *data, uint8_t len, uint8_t rw_flag);

#endif  // _RTC_H