#include "button.h"
#include "config.h"

#define DEBOUNCE_DELAY_MS    5
#define DOUBLECLICK_SCAN_CYCLES 370

#if (DAWNALARM_MK == 1)
#   define BUTTON_PULL  0
#elif (DAWNALARM_MK == 2)
#   define BUTTON_PULL  1
#endif

static bool last_state = FALSE;

bool btn_is_pressed(void)
{
    return !!(BUTTON_GPORT->IDR & BUTTON_GPIN) != BUTTON_PULL;
}

bool btn_pressed(void)
{
    bool pressed = FALSE;
    bool current_state = !!(BUTTON_GPORT->IDR & BUTTON_GPIN);

    if (last_state != current_state) {
        delay_ms(DEBOUNCE_DELAY_MS);
        current_state = !!(BUTTON_GPORT->IDR & BUTTON_GPIN);
    }

    if (!last_state && current_state)
        pressed = !BUTTON_PULL;
    else if (last_state && !current_state)
        pressed = BUTTON_PULL;
    last_state = current_state;
    return pressed;
}

bool btn_pressed_again(void)
{
    uint16_t i;

    for (i = 0; i < DOUBLECLICK_SCAN_CYCLES; i++) {
        delay_ms(1);
        if (btn_pressed()) {
            return TRUE;
        }
    }
    return FALSE;
}