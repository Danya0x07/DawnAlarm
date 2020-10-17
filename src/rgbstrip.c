#include "rgbstrip.h"

void rgbstrip_set(enum color c, uint8_t val)
{
    switch (c)
    {
    case COLOR_RED:   rgbstrip_set_R(val); break;
    case COLOR_GREEN: rgbstrip_set_G(val); break;
    case COLOR_BLUE:  rgbstrip_set_B(val); break;
    }
}

void rgbstrip_kill(void)
{
    rgbstrip_set_R(0);
    rgbstrip_set_G(0);
    rgbstrip_set_B(0);
}
