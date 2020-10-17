#ifndef _UI_H
#define _UI_H

#include "halutils.h"
#include "rgbstrip.h"

enum menu_item {
    ITEM_ALARMSETUP,
    ITEM_COLORSETUP,
    ITEM_DISKO,
    ITEM_CLOCKSETUP,
    ITEM_CANCEL,

    ITEMS_TOTAL
};

void ui_show_splash_screen(void);
enum menu_item ui_get_user_menu_item(void);
uint16_t ui_get_user_time(uint16_t current_time, bool dots);
uint8_t ui_get_user_dawn_duration(void);
void ui_set_strip_colors_brightness(void);
void ui_perform_disko(void);

#endif  // _UI_H