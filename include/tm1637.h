#ifndef TM1637_H_INCLUDED
#define TM1637_H_INCLUDED

#include "halutils.h"

#define TM_DEFAULT_BRIGHTNESS   7
#define TM_NIGHT_BRIGHTNESS     0

/* Карта разряда:
 *     A
 *    ---
 * F |   | B
 *    -G-
 * E |   | C
 *    ---
 *     D
 * При необходимости перечисление может быть дополнено
 * пользовательскими символами. Для 2-го слева разряда дисплея
 * бит Х отвечает за двоеточие.
 */
enum {    // XGFEDCBA
    TM_0 = 0b00111111,
    TM_1 = 0b00000110,
    TM_2 = 0b01011011,
    TM_3 = 0b01001111,
    TM_4 = 0b01100110,
    TM_5 = 0b01101101,
    TM_6 = 0b01111101,
    TM_7 = 0b00000111,
    TM_8 = 0b01111111,
    TM_9 = 0b01101111,
    TM_A = 0b01110111,
    TM_b = 0b01111100,
    TM_C = 0b00111001,
    TM_d = 0b01011110,
    TM_E = 0b01111001,
    TM_F = 0b01110001,
    TM_H = 0b01110110,
    TM_h = 0b01110100,
    TM_I = 0b00110000,
    TM_L = 0b00111000,
    TM_o = 0b01011100,
    TM_P = 0b01110011,
    TM_q = 0b01100111,
    TM_r = 0b00110001,
    TM_U = 0b00111110,
    TM_y = 0b01101110,
    TM_MINUS  = 0b01000000,
    TM_UNDER  = 0b00001000,
    TM_ABOVE  = 0b00000001,
    TM_DEGREE = 0b01100011,
    TM_DOTS   = 0b10000000,
    TM_CLEAR  = 0b00000000,
};

/* Массив для перевода чисел [0; 0xF] в "кодировку" дисплея.
 * Нужен для того чтобы вывести на дисплей одновременно цифры
 * и нецифровые символы (с помощью функции tm1637_display_content),
 * например:
 *  uint8_t hp = ...
 *  ...
 *  uint8_t content[4] = {TM_H, TM_P, tm_digits[hp / 10], tm_digits[hp % 10]};
 *  tm1637_display_content(content); */
extern const uint8_t tm_digits[0x10];

/* Отображает десятичное число number [-999; 9999] на дисплее,
 * dots == 1: отображать двоеточие, 0: не отображать. */
void tm1637_display_dec(int16_t number, bool dots);

/* TODO: tm1637_display_hex( ) хорошо бы иметь,
 * но в этом проекте в ней пока нет необходимости. */

/* Отображает последовательность из 4х пользовательских символов.
 * Пример:
 *  uint8_t content[4] = {TM_y, TM_0 | TM_DOTS, TM_U, TM_r}; // yo:Ur
 *  tm1637_display_content(content); */
void tm1637_display_content(uint8_t content[4]);

/* Переключает состояние дисплея,
 * displaying == 1: сегменты светятся, 0: не светятся. */
void tm1637_set_displaying(bool displaying);

/* Устанавливает яркость дисплея.
 * brightness = [0; 7]
 * Побочный эффект: включает дисплей, если он был выключен. */
void tm1637_set_brightness(uint8_t brightness);

/* Возвращает текущую яркость дисплея [0; 7].
 * Даже после tm1637_set_displaying(FALSE). */
uint8_t tm1637_get_brightness(void);

#endif
