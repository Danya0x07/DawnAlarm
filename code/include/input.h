#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <stm8s.h>

void input_setup(void);

bool btn_pressed(void);
bool btn_pressed_again(void);
bool btn_is_pressed(void);
/* scale: размерность возвращаемого значения.
 * Например, для ввода значения часов - 23, минут - 59. */
uint16_t potentiometer_get(uint16_t scale);

#endif
