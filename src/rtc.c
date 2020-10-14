#include "rtc.h"

#define DS1307_I2C_ADDRESS  0b11010000

#define DS1307_MEMSTART_ADDR    0x00
#define DS1307_NVRAMSTART_ADDR  0x08
#define DS1307_NVRAMEND_ADDR    0x3F
#define NVRAM_LEN   (DS1307_NVRAMEND_ADDR - DS1307_NVRAMSTART_ADDR + 1)
#define NVRAM_SHIFT 0  // Сдвиг адреса для распределения изнашиваемости ячеек.

#define BIT_CH   7
#define BIT_OUT  7
#define BIT_SQWE 4
#define BIT_RS1  1
#define BIT_RS0  0

static uint8_t convert_to_bindec(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t convert_from_bindec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0F);
}

void rtc_setup(uint16_t time)
{
    uint8_t settings[] = {
        DS1307_MEMSTART_ADDR,  // начало памяти
        0,  // запуск счёта времени | секунды
        convert_to_bindec(time % 100),  // минуты
        convert_to_bindec(time / 100),  // часы
        1,  // день недели (не используется)
        1,  // число (не используется)
        1,  // месяц (не используется)
        0,  // год (не используется)
        1 << BIT_SQWE  // вывод секунд на пин
    };
    i2c_write_bytes(DS1307_I2C_ADDRESS, settings, sizeof(settings), 0);
}

bool rtc_is_running(void)
{
    uint8_t settings;
    uint8_t mem_addr = DS1307_MEMSTART_ADDR;

    i2c_write_bytes(DS1307_I2C_ADDRESS, &mem_addr, 1, I2C_NOSTOP);
    i2c_read_bytes(DS1307_I2C_ADDRESS, &settings, 1, 0);

    return (settings & (1 << BIT_CH)) == 0;
}

uint16_t rtc_get_time(void)
{
    uint8_t current_time[2] = {0};
    uint8_t mem_addr = 0x01;

    i2c_write_bytes(DS1307_I2C_ADDRESS, &mem_addr, 1, I2C_NOSTOP);
    i2c_read_bytes(DS1307_I2C_ADDRESS, current_time, sizeof(current_time), 0);
    current_time[0] = convert_from_bindec(current_time[0]);
    current_time[1] = convert_from_bindec(current_time[1]);
    return (uint16_t) current_time[1] * 100 + current_time[0];
}

void rtc_set_time(uint16_t time)
{
    uint8_t settings[] = {
        DS1307_MEMSTART_ADDR,  // начало памяти
        0,  // секунды
        convert_to_bindec(time % 100),  // минуты
        convert_to_bindec(time / 100),  // часы
    };
    i2c_write_bytes(DS1307_I2C_ADDRESS, settings, sizeof(settings), 0);
}

void rtc_access_nvram(uint8_t *data, uint8_t len, uint8_t rw_flag)
{
    uint8_t save_addr = DS1307_NVRAMSTART_ADDR + NVRAM_SHIFT;

    if ((uint16_t)save_addr + len > DS1307_NVRAMEND_ADDR)
        return;

    i2c_write_bytes(DS1307_I2C_ADDRESS, &save_addr, 1, I2C_NOSTOP);
    
    if (rw_flag)
        i2c_read_bytes(DS1307_I2C_ADDRESS, data, len, 0);
    else
        i2c_write_bytes(DS1307_I2C_ADDRESS, data, len, I2C_NOSTART);
}