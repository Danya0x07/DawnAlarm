#ifndef TM1637_H_INCLUDED
#define TM1637_H_INCLUDED

#include <stm8s.h>

/* Яркость сегментов [0; 7]. */
#define TM_BRIGHTNESSS  5

enum tm_charset {
    TM_0, TM_1, TM_2, TM_3, TM_4, TM_5, TM_6, TM_7,
    TM_8, TM_9, TM_A, TM_b, TM_C, TM_d, TM_E, TM_F,
    TM_H, TM_I, TM_L, TM_P, TM_q, TM_r, TM_U,
    TM_MINUS, TM_UNDERSCORE, TM_DEGREE, TM_CLEAR,
    TM_CHARS_TOTAL
};

void tm1637_gpio_setup(void);

/* Отображает десятичное число number [-999; 9999] на дисплее,
 * dots == 1: отображать двоеточие, 0: не отображать. */
void tm1637_display_dec(int16_t number, bool dots);

/* Отображает последовательность из 4х символов tm_charset,
 * dots == 1: отображать двоеточие, 0: не отображать. */
void tm1637_display_chars(const enum tm_charset[4], bool dots);

/* Переключает состояние дисплея,
 * displaying == 1: сегменты светятся, 0: не светятся. */
void tm1637_set_displaying(bool displaying);

#endif
