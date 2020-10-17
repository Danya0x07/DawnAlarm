#include "ui.h"
#include "button.h"
#include "selector.h"
#include "tm1637.h"
#include "config.h"

static int8_t parse_number_into_tm16_digits(int16_t number, uint8_t radix, uint8_t *digits)
{
    int16_t i;
    uint8_t digit;
    int8_t len = 0;
    bool started = FALSE;

    if ((radix == 10 && (number > 9999 || number < -999)) ||
        (radix == 16 && (number < -0x0FFF)))
            return -1;

    if (number < 0) {
        *digits++ = TM16_MINUS;
        number = -number;
    } 

    if (radix == 10) {
        for (i = 1000; i > 0; i /= 10) {
            digit = number / i;
            if (digit > 0 || started) {
                started = TRUE;
                *digits++ = tm16_digits[digit];
                number -= i * digit;
                len++;
            }
        }
    }
    else if (radix == 16) {
        for (i = 12; i >= 0; i -= 4) {
            digit = (number >> i) & 0x0F;
            if (digit > 0 || started) {
                started = TRUE;
                *digits++ = tm16_digits[digit];
                len++;
            }
        }
    }

    if (len == 0) {
        *digits++ = tm16_digits[0];
        len = 1;
    }
    return len;
}

static void display_value_with_caption(const uint8_t *caption, int16_t value, uint8_t radix, 
                                       uint8_t startpos)
{
    static uint8_t content_buff[4];
    static uint8_t digit_buff[4];
    uint8_t *cb = content_buff;
    int8_t len;
    uint8_t i;

    for (i = 0; i < startpos; i++)
        *cb++ = *caption++;

    if ((len = parse_number_into_tm16_digits(value, radix, digit_buff)) < 0)
        return;
    
    for (i = 0; i < (uint8_t)len; i++)
        *cb++ = digit_buff[i];

    for (i = startpos + len; i < 4; i++)
        *cb++ = *caption++;

    tm1637_display_content(content_buff);
}

static int16_t get_user_value(int16_t val_min, int16_t val_max, int16_t val_initial, 
                               void (*callback)(int16_t, void *), void *additional_arg)
{
    int16_t val = 0, prev_val = (int16_t)0xFFFF;

    selector_set(val_min, val_max, val_initial);
    selector_irq_on();
    while (!button_pressed()) {
        val = selector_get();
        if (val != prev_val) {
            callback(val, additional_arg);
            prev_val = val;
        }
        delay_ms(10);
    }
    selector_irq_off();
    return val;
}

static void cb_get_menu_item(int16_t item, void *unused)
{
    static const uint8_t menu[ITEMS_TOTAL][4] = {
        {TM16_A, TM16_L, TM16_A, TM16_r},
        {TM16_CLEAR, TM16_C, TM16_0, TM16_L},
        {TM16_d, TM16_I, TM16_C, TM16_0},
        {TM16_C, TM16_L, TM16_0, TM16_C},
        {TM16_0, TM16_E, TM16_H, TM16_A}
    };
    tm1637_display_content(menu[item]);
}

static void cb_get_user_time(int16_t val1, void *pval2)
{
    uint8_t hours, minutes;
    uint8_t data = *((uint8_t *)pval2);
    bool dots = !!(data & 0x80);

    if (data & 0x40) {  // в val1 данные о минутах, в *pval2 - о часах и двоеточии
        hours = data & 0x3F;
        minutes = val1;
    } else {  // в val1 данные о часах, в *pval2 - о минутах и двоеточии
        hours = val1;
        minutes = data & 0x3F;
    }
    tm1637_display_dec(hours * 100 + minutes, dots);
}

static void cb_get_user_dd(int16_t dd, void *out)
{
    static const uint8_t caption[4] = {TM16_d, TM16_d, TM16_0, TM16_0};
    
    if (dd == 1) { 
        dd = 5;
    }
    else {
        dd = (dd - 1) * 10;
    }
    display_value_with_caption(caption, dd, 10, dd > 9 ? 2 : 3);
    *((uint8_t *)out) = dd;
}

static void cb_display_brightness(int16_t value, void *color)
{
    static const uint8_t captions[3][4] =  {
        {TM16_r, TM16_E, TM16_d, TM16_0},
        {TM16_9, TM16_r, TM16_E, TM16_0},
        {TM16_b, TM16_L, TM16_U, TM16_0}
    };
    enum color col = *((enum color *)color);
    
    display_value_with_caption(captions[col], value, 16, 3);
    value = (0xFF * value) >> 4;
    rgbstrip_set(col, value);
}

static void cb_get_user_boolean(int16_t value, void *unused)
{
    static const uint8_t options[2][4] = {
        {TM16_CLEAR, TM16_o, TM16_F, TM16_F},
        {TM16_CLEAR, TM16_CLEAR, TM16_o, TM16_n}
    };
    tm1637_display_content(options[value & 1]);
}

void ui_show_splash_screen(void)
{
    static const uint8_t splash_screen[4] = 
        {TM16_MINUS, TM16_MINUS | TM16_DOTS, TM16_MINUS, TM16_MINUS};
    tm1637_display_content(splash_screen);
}

enum menu_item ui_get_user_menu_item(void)
{
    return get_user_value(0, ITEMS_TOTAL - 1, 0, cb_get_menu_item, NULL);
}

uint16_t ui_get_user_time(uint16_t current_time, bool dots)
{
    uint8_t hours = 0, minutes = 0;
    uint8_t tmp = minutes | (dots << 7);

    hours = get_user_value(0, 23, current_time / 100, cb_get_user_time, &tmp);

    tmp = hours | (dots << 7) | 0x40;
    minutes = get_user_value(0, 59, current_time % 100, cb_get_user_time, &tmp);

    return hours * 100 + minutes;
}

uint8_t ui_get_user_dawn_duration(void)
{
    uint8_t dd;
    get_user_value(1, 7, 4, cb_get_user_dd, &dd);
    return dd;
}

void ui_set_strip_colors_brightness(void)
{
    uint8_t i;
    enum color c;
    for (i = 0; i < 3; i++) {
        c = (enum color)i;
        get_user_value(0, 0x0F, 0, cb_display_brightness, &c);
    }
}

bool ui_get_user_boolean(void)
{
    return get_user_value(0, 1, 0, cb_get_user_boolean, NULL);
}

static enum color disko_change_color(enum color c)
{
    switch (c)
    {
    case COLOR_RED:   return COLOR_GREEN;
    case COLOR_GREEN: return COLOR_BLUE;
    case COLOR_BLUE:  return COLOR_RED;
    default: return COLOR_RED;
    }
}

void ui_perform_disko(void)
{
    enum color incr_color = COLOR_GREEN;
    enum color decr_color = COLOR_RED;
    static const uint8_t smiley[4] = {0x18, 0xEB, 0x6B, 0x0C};
    uint16_t i;

    tm1637_display_content(smiley);
    while (button_is_pressed());
    delay_ms(10);  // кнопочный дребезг

    while (!button_is_pressed()) {
        for (i = 0; i < RGB_MAX_VALUE + 1 && !button_is_pressed(); i++) {
            rgbstrip_set(incr_color, i);
            rgbstrip_set(decr_color, RGB_MAX_VALUE - i);
            delay_ms(20);
        }
        decr_color = incr_color;
        incr_color = disko_change_color(incr_color);
    }
    rgbstrip_set_R(0);
    rgbstrip_set_G(0);
    rgbstrip_set_B(0);
}