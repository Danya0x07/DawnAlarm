#ifndef _BATTERY
#define _BATTERY

#include "halutils.h"

#if (DAWNALARM_MK == 2)
uint8_t battery_get_charge(void);
bool battery_level_is_low(void);
#endif

#endif  // _BATTERY_H