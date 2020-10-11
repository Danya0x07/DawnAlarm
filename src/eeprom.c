#include "eeprom.h"

#define AT24C32_ADDRESS 0b10100000

void eeprom_save(struct options *opts)
{
    /*
     * Поле __eeprom_addr воспринимается микросхемой как адрес, куда записывать
     * остальную часть структуры, т.е. фактически оно не осхраняется в eeprom.
     */
    i2c_write_bytes(AT24C32_ADDRESS, (uint8_t *)opts, sizeof(*opts), 0);
}

void eeprom_load(struct options *opts)
{
    /*
     * Сначала записываем адрес, откуда считывать, а далее считываем данные
     * в структуру, начиная с этого адреса.
     */
    i2c_write_bytes(AT24C32_ADDRESS, (const uint8_t *)&opts->__eeprom_addr, 
                    sizeof(opts->__eeprom_addr), I2C_NOSTOP);
    i2c_read_bytes(AT24C32_ADDRESS, (uint8_t *)opts + sizeof(opts->__eeprom_addr),
                   sizeof(*opts) - sizeof(opts->__eeprom_addr), 0);
}
