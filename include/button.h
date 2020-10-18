#ifndef _BUTTON_H
#define _BUTTON_H

#include "halutils.h"
#include "config.h"

#define button_irq_on()     (BUTTON_GPORT->CR2 |= BUTTON_GPIN)
#define button_irq_off()    (BUTTON_GPORT->CR2 &= ~BUTTON_GPIN)

/**
 * Возвращает TRUE, если кнопка была переведена из ненажатого состояния в
 * нажатое.
 * После первого срабатывания, вернувшего TRUE, возвращает FALSE до тех пор,
 * пока кнопка не будет отпущена и нажата снова.
 */
bool button_pressed(void);

/** Возвращает TRUE, если кнопка зажата в данный момент, иначе FALSE. */
bool button_is_pressed(void);

#endif  // _BUTTON_H