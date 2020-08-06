#ifndef AT24C32_H_INCLUDED
#define AT24C32_H_INCLUDED

#include <stm8s.h>

/* Настройки, которые необходимо хранить в энергонезависимой памяти. */
struct options {
    uint16_t __eeprom_addr; // адрес места сохранения в памяти микросхемы eeprom
    uint16_t alarm_time;    // время полного рассвета
    uint8_t  dawn_duration; // длительность рассвета (в минутах)
};

void eeprom_save(struct options *opts);
void eeprom_load(struct options *opts);

#endif
