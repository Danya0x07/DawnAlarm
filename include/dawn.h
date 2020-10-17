#ifndef _DAWN_H
#define _DAWN_H

#ifdef UNIT_TEST
#   include <stdint.h>
#   include <stdbool.h>
#else
#   include "halutils.h"
#endif

void dawn_setup(int16_t alarm_time, uint16_t dawn_duration);
void dawn_update(int16_t current_time);
bool dawn_is_ongoing(int16_t current_time);

#endif  // _DAWN_H