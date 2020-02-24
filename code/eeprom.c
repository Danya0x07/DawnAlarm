#include "eeprom.h"
#include "utils.h"

#define AT24C32_ADDRESS 0b10100000

void eeprom_save(struct options *opts)
{
    /* Поле __eeprom_addr воспринимается микросхемой как адрес, куда записывать
     * остальную часть структуры, т.е. фактически оно не осхраняется в eeprom. */
    i2c_mem_write(AT24C32_ADDRESS, (uint8_t *)opts, sizeof(*opts));
}

void eeprom_load(struct options *opts)
{
    /* Сначала записываем адрес, откуда считывать, а далее считываем данные
     * в структуру, начиная с этого адреса. */
    i2c_set_mem_ptr(AT24C32_ADDRESS, (uint8_t *)&opts->__eeprom_addr, sizeof(opts->__eeprom_addr));
    i2c_mem_read(AT24C32_ADDRESS, (uint8_t *)opts + sizeof(opts->__eeprom_addr),
                 sizeof(*opts) - sizeof(opts->__eeprom_addr));
}
