#include "dawn.h"
#include "rgbstrip.h"

#define DAWN_DURATION_SPANS 5
#define GREEN_DAWN_PART     4
#define BLUE_DAWN_PART      3

#define MINUTES_PER_DAY (60 * 24)

static struct dawn {
    int16_t start_time;
    uint8_t duration;
} red_dawn, green_dawn, blue_dawn;

static int16_t abs_alarm_time;

static int16_t get_abs_time(int16_t time)
{
    uint8_t hours = time / 100;
    uint8_t minutes = time - 100 * hours;
    return (int16_t)60 * hours + minutes;
}

void dawn_setup(int16_t alarm_time, uint16_t dawn_duration)
{
    if (dawn_duration == 0)
        dawn_duration = 1;
    /*
     * Яркость должна набираться со смещением: сначала красный, потом зелёный и синий.
     * При этом синий должен закончить набирать яркость ровно к установленному моменту подъёма.
     * 
     * Red     Green  Blue                RGB
     * start   start  start               finish (alarm_time)
     * |-------|------|-------------------|
     * |                                  |
     * |        dawn_duration             |
     * |<-------------------------------->|
     */
    red_dawn.duration = dawn_duration;
    green_dawn.duration = dawn_duration * GREEN_DAWN_PART / DAWN_DURATION_SPANS;
    blue_dawn.duration = dawn_duration * BLUE_DAWN_PART / DAWN_DURATION_SPANS;

    abs_alarm_time = get_abs_time(alarm_time);

    red_dawn.start_time = abs_alarm_time - red_dawn.duration + 1;
    green_dawn.start_time = abs_alarm_time - green_dawn.duration + 1;
    blue_dawn.start_time = abs_alarm_time - blue_dawn.duration + 1;
}

static void color_update(const struct dawn *dawn, enum color color, 
                         int16_t current_time)
{
    uint16_t brightness;
    int16_t dawn_current_minute;

    current_time = get_abs_time(current_time);
    /*
     * Защита от случаев, когда будильник установили на 00:ХХ,
     * где ХХ < длительности рассвета.
     */
    if (abs_alarm_time < dawn->duration && current_time > abs_alarm_time) {
        current_time -= MINUTES_PER_DAY;
    }

    dawn_current_minute = current_time - dawn->start_time + 1;
    if (dawn_current_minute <= 0 || dawn_current_minute > dawn->duration)
        return; 
    /* 
     * dawn_current_minute      color_brightness
     * -------------------  ==  ----------------
     * dawn.duration            max_brightness
     */
    brightness = dawn_current_minute * RGB_MAX_VALUE / dawn->duration;
    rgbstrip_set(color, brightness);
}

void dawn_update(int16_t current_time)
{
    color_update(&red_dawn, COLOR_RED, current_time);
    color_update(&green_dawn, COLOR_GREEN, current_time);
    color_update(&blue_dawn, COLOR_BLUE, current_time);
}

bool dawn_is_ongoing(int16_t current_time)
{
    current_time = get_abs_time(current_time);
    if (abs_alarm_time < red_dawn.duration && current_time > abs_alarm_time) {
        current_time -= MINUTES_PER_DAY;
    }
    return current_time >= red_dawn.start_time && current_time <= abs_alarm_time;
}