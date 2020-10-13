#ifndef _HALUTILS_H
#define _HALUTILS_H

#include <stm8s.h>
#include <stddef.h>

#define tim4_set_counter(value)     (TIM4->CNTR = (value))
#define tim4_get_counter()  (TIM4->CNTR)

void delay_ms(uint16_t ms);

#define I2C_NOSTART (1 << 0)
#define I2C_NOSTOP  (1 << 1)

void i2c_write_bytes(uint8_t addr, const uint8_t *data, uint8_t len, uint8_t flags);
void i2c_read_bytes(uint8_t addr, uint8_t *data, uint8_t len, uint8_t flags);

#endif  // _HALUTILS_H