#include "button.h"
#include "config.h"

#define DEBOUNCE_DELAY_MS    5

#if (DAWNALARM_MK == 1)
#   define BUTTON_PULL  0
#elif (DAWNALARM_MK == 2)
#   define BUTTON_PULL  1
#endif

static bool last_state = FALSE;

static bool get_btn_pin_state(void)
{
    return !!(BUTTON_GPORT->IDR & BUTTON_GPIN);
}

bool button_is_pressed(void)
{
    return get_btn_pin_state() != BUTTON_PULL;
}

bool button_pressed(void)
{
    bool pressed = FALSE;
    bool current_state = get_btn_pin_state();

    if (last_state != current_state) {
        delay_ms(DEBOUNCE_DELAY_MS);
        current_state = get_btn_pin_state();
    }

    if (!last_state && current_state)
        pressed = !BUTTON_PULL;
    else if (last_state && !current_state)
        pressed = BUTTON_PULL;
    last_state = current_state;
    return pressed;
}