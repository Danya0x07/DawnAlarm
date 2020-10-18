#ifndef _BATTERY
#define _BATTERY

#include "halutils.h"

uint8_t battery_get_charge(void);
bool battery_level_is_low(void);

#endif  // _BATTERY_H