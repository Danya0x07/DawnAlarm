#ifndef _UI_H
#define _UI_H

#include "halutils.h"
#include "rgbstrip.h"

enum menu_item {
    ITEM_ALARMSETUP,
#if (DAWNALARM_MK == 2)
    ITEM_BUZZERSETUP,
#endif
    ITEM_COLORSETUP,
    ITEM_DISKO,
    ITEM_CLOCKSETUP,
#if (DAWNALARM_MK == 2)
    ITEM_CHARGE,
#endif
    ITEM_CANCEL,

    ITEMS_TOTAL
};

void ui_show_splash_screen(void);
enum menu_item ui_get_user_menu_item(void);
uint16_t ui_get_user_time(uint16_t current_time, bool dots);
uint8_t ui_get_user_dawn_duration(void);
void ui_set_strip_colors_brightness(void);
bool ui_get_user_boolean(void);
void ui_perform_disko(void);

#if (DAWNALARM_MK == 2)
void ui_show_battery_level_low(void);
void ui_show_charge_level(void);
bool ui_get_user_buzzer_status(void);
#endif

#endif  // _UI_H