#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stm8s.h>

void delay_ms(uint16_t);
void __delay_irq_handler(void) __interrupt(23);

/* Записывает блок памяти data размером count байт
 * в устройство с адресом slave_addr. */
void i2c_mem_write(uint8_t slave_addr, uint8_t *data, uint8_t count);

/* Принимает count байт в массив buffer от устройства с адресом slave_addr. */
void i2c_mem_read(uint8_t slave_addr, uint8_t *buffer, uint8_t count);

/* Устанавливает адрес памяти mem_addr длиной addr_size, с которого будет
 * происходить запись/чтение в устройстве с адресом slave_addr. */
void i2c_set_mem_ptr(uint8_t slave_addr, uint8_t *mem_addr, uint8_t addr_size);

#endif
