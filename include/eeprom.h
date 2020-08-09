#ifndef _AT24C32_H
#define _AT24C32_H

#include "halutils.h"

/*
 * Настройки, которые необходимо хранить в энергонезависимой памяти,
 * (кроме __eeprom_addr).
 */
struct options {
    uint16_t __eeprom_addr;  // адрес места сохранения в памяти микросхемы
    uint16_t alarm_time;     // время полного рассвета
    uint8_t  dawn_duration;  // длительность рассвета (в минутах)
};

void eeprom_save(struct options *opts);
void eeprom_load(struct options *opts);

#endif  // _AT24C32_H