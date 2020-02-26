#include "rgbtape.h"
#include "utils.h"

void rgbtape_set(enum color c, uint8_t val)
{
    switch (c)
    {
    case COLOR_RED:   rgbtape_set_R(val); break;
    case COLOR_GREEN: rgbtape_set_G(val); break;
    case COLOR_BLUE:  rgbtape_set_B(val); break;
    }
}

enum color rgbtape_change_color(enum color c)
{
    switch (c)
    {
    case COLOR_RED:   return COLOR_GREEN;
    case COLOR_GREEN: return COLOR_BLUE;
    case COLOR_BLUE:  return COLOR_RED;
    default: return COLOR_RED;
    }
}
