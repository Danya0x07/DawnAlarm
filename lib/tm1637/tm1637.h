/*
 * Copyright (C) 2020 Daniel Efimenko
 *     github.com/Danya0x07
 */

/*----------------------------------------------------------
 * A simple and cross-platform library 
 * for interfacing TM1637 LED drive control special cirquit.
 *---------------------------------------------------------*/

#ifndef _TM1637_H
#define _TM1637_H

//#include <stdint.h>
//#include <stdbool.h>
#include <stm8s.h>

/* The segment map:
 *     A
 *    ---
 * F |   | B
 *    -G-
 * E |   | C
 *    ---
 *     D
 * The enumeration can be extended with user symbols. 
 * For the 2nd (counting from left, [12:34]) symbol the X bit controls dots.
 */
enum tm1637_bitmap {    // XGFEDCBA
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
    TM_DOTS   = 0b10000000,  // only for 2nd symbol
    TM_CLEAR  = 0b00000000,
};

/* Array for converting one-digit numbers(0..0xF) into TM1637 symbols.
 * Needed for displaying numbers with some caption (like [-00-] or [PH:23])
 * Example:
 *  int hp = 75;
 *  ...
 *  uint8_t content[4] = 
 *      {TM_H, TM_P | TM_DOTS, tm_digits[hp / 10], tm_digits[hp % 10]};
 *  tm1637_display_content(content);  // The segments will look like [HP:75].
 */
extern const uint8_t tm_digits[0x10];

/* Display a decimal number (-999..9999) on the segments
 * displaying dots(1) or not(0). */
void tm1637_display_dec(int16_t number, bool dots);

/* Display some user content(uint8_t[4]) on the segments.
 * Example:
 *  uint8_t content[4] = {TM_y, TM_o | TM_DOTS, TM_U, TM_r}; // yo:Ur
 *  tm1637_display_content(content); */
void tm1637_display_content(uint8_t content[4]);

/* Set the display state on(1) or off(0). */
void tm1637_set_state(bool state);

/* Get the display state (on(1) or off(0)). */
bool tm1637_get_state(void);

/* Set the segments brightness(0..7).
 * Side effect: if the display was turned off then it turns on. */
void tm1637_set_brightness(uint8_t brightness);

/* Get the segments brightness(0..7). */
uint8_t tm1637_get_brightness(void);

/* TODO: 
 *  Add tm1637_display_hex();
 *  Implement key reading.
 */

#endif  // _TM1637_H