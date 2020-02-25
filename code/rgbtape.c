#include "rgbtape.h"

void rgbtape_set(enum color col, uint8_t value)
{
    switch (col)
    {
    case COLOR_RED:   rgbtape_set_R(value); break;
    case COLOR_GREEN: rgbtape_set_G(value); break;
    case COLOR_BLUE:  rgbtape_set_B(value); break;
    }
}
