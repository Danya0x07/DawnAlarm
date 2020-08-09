#include "ds1307.h"

#define DS1307_ADDRESS  0b11010000
#define DS_CH   (1 << 7)
#define DS_OUT  (1 << 7)
#define DS_SQWE (1 << 4)
#define DS_RS1  (1 << 1)
#define DS_RS0  (1 << 0)

static uint8_t convert_to_bindec(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t convert_from_bindec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0F);
}

void ds1307_setup(uint16_t time)
{
    uint8_t settings[] = {
        0,  // начало памяти
        0,  // запуск счёта времени | секунды
        convert_to_bindec(time % 100),  // минуты
        convert_to_bindec(time / 100),  // часы
        1,  // день недели (не используется)
        1,  // число (не используется)
        1,  // месяц (не используется)
        0,  // год (не используется)
        DS_SQWE  // вывод секунд на пин
    };
    i2c_mem_write(DS1307_ADDRESS, settings, sizeof(settings));
}

uint16_t ds1307_get_time(void)
{
    uint8_t current_time[2] = {0};
    uint8_t mem_address = 0x01;

    i2c_set_mem_ptr(DS1307_ADDRESS, &mem_address, sizeof(mem_address));
    i2c_mem_read(DS1307_ADDRESS, current_time, sizeof(current_time));
    current_time[0] = convert_from_bindec(current_time[0]);
    current_time[1] = convert_from_bindec(current_time[1]);
    return (uint16_t) current_time[1] * 100 + current_time[0];
}

void ds1307_set_time(uint16_t time)
{
    uint8_t settings[] = {
        0,  // начало памяти
        0,  // секунды
        convert_to_bindec(time % 100),  // минуты
        convert_to_bindec(time / 100),  // часы
    };
    i2c_mem_write(DS1307_ADDRESS, settings, sizeof(settings));
}
