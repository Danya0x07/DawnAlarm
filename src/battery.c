#include "battery.h"
#include "config.h"

// Значения сопротивлений делителя напряжения в сотнях Ом
#define R1  22
#define R2  82

// Граничные напряжения аккумулятора в вольтах/10
#define BATTERY_MAX_VOLTAGE 41
#define BATTERY_MIN_VOLTAGE 28

#define CRITICAL_CHARGE_LEVEL   5

uint8_t battery_get_charge(void)
{
    int32_t charge;

    adc_start_conversion();
    while (!adc_conversion_complete()) {}
    charge = adc_read_value();
    /*
     *  ADC       voltage * 10
     * ------ == --------------
     *  1023        33
     */
    charge = charge * 33 / 1023;  // напряжение на входе в вольтах/10
    /*
     *  Vin       R1 + R2
     * ------ == ---------
     *  Vout        R2
     */
    charge = charge * (R1 + R2) / R2;  // напряжение батарейки в вольтах/10
    /* 
     *        charge - BATTERY_MIN_VOLTAGE              percentage
     * ------------------------------------------- == -------------
     * (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)         100
     */
    charge = (charge - BATTERY_MIN_VOLTAGE) * 100 / 
                (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);
    if (charge < 0)
        charge = 0;
    if (charge > 100)
        charge = 100;
    return (uint8_t)charge;
}

bool battery_level_is_low(void)
{
    return battery_get_charge() <= CRITICAL_CHARGE_LEVEL;
}