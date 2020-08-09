#ifndef _HALUTILS_H
#define _HALUTILS_H

#include <stm8s.h>

#define tim4_set_counter(value)     (TIM4->CNTR = (value))
#define tim4_get_counter()  (TIM4->CNTR)

/* Миллисекундная задержка. */
void delay_ms(uint16_t);

/*
 * Записывает блок памяти data размером count байт
 * в устройство с адресом slave_addr.
 */
void i2c_mem_write(uint8_t slave_addr, uint8_t *data, uint8_t count);

/* Принимает count байт в массив buffer от устройства с адресом slave_addr. */
void i2c_mem_read(uint8_t slave_addr, uint8_t *buffer, uint8_t count);

/*
 * Устанавливает адрес памяти mem_addr длиной addr_size, с которого будет
 * происходить запись/чтение в устройстве с I2C адресом slave_addr.
 */
void i2c_set_mem_ptr(uint8_t slave_addr, uint8_t *mem_addr, uint8_t addr_size);

#endif  // _HALUTILS_H