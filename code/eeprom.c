#include "eeprom.h"
#include "utils.h"

#define AT24C32_ADDRESS 0b10100000

void eeprom_save(struct options* opts)
{
    i2c_mem_write(AT24C32_ADDRESS, (uint8_t*) opts, sizeof(struct options));
}

void eeprom_load(struct options* opts)
{
    i2c_set_mem_ptr(AT24C32_ADDRESS, (uint8_t*) &opts->__eeprom_addr, sizeof(opts->__eeprom_addr));
    i2c_mem_read(AT24C32_ADDRESS, ((uint8_t*) opts) + 2, sizeof(struct options) - 2);
}
