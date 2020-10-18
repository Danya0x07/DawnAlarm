#ifndef _HALUTILS_H
#define _HALUTILS_H

#include <stm8s.h>
#include <stddef.h>

#define tim4_set_counter(value)     (TIM4->CNTR = (value))
#define tim4_get_counter()  (TIM4->CNTR)

void delay_ms(uint16_t ms);

void adc_start_conversion(void);
bool adc_conversion_complete(void);
uint16_t adc_read_value(void);

#define I2C_NOSTART (1 << 0)
#define I2C_NOSTOP  (1 << 1)

void i2c_write_bytes(uint8_t addr, const uint8_t *data, uint8_t len, uint8_t flags);
void i2c_read_bytes(uint8_t addr, uint8_t *data, uint8_t len, uint8_t flags);

/* Скопировано без изменений (за искл. форматирования и названия) из STM8S_StdPeriph_Driver.
 * Поскольку компилятор SDCC пришивает библиотеки целиком, даже если
 * из неё используется всего несколько функций, вынос используемого
 * кода в отдельный файл позволит значительно сэкономить память МК.
 */

void _I2C_Init(uint32_t OutputClockFrequencyHz, uint16_t OwnAddress,
              I2C_DutyCycle_TypeDef I2C_DutyCycle, I2C_Ack_TypeDef Ack,
              I2C_AddMode_TypeDef AddMode, uint8_t InputClockFrequencyMHz);

void _TIM1_TimeBaseInit(uint16_t TIM1_Prescaler,
                       TIM1_CounterMode_TypeDef TIM1_CounterMode,
                       uint16_t TIM1_Period,
                       uint8_t TIM1_RepetitionCounter);

void _TIM1_OC1Init(TIM1_OCMode_TypeDef TIM1_OCMode,
                  TIM1_OutputState_TypeDef TIM1_OutputState,
                  TIM1_OutputNState_TypeDef TIM1_OutputNState,
                  uint16_t TIM1_Pulse,
                  TIM1_OCPolarity_TypeDef TIM1_OCPolarity,
                  TIM1_OCNPolarity_TypeDef TIM1_OCNPolarity,
                  TIM1_OCIdleState_TypeDef TIM1_OCIdleState,
                  TIM1_OCNIdleState_TypeDef TIM1_OCNIdleState);

void _TIM1_OC2Init(TIM1_OCMode_TypeDef TIM1_OCMode,
                  TIM1_OutputState_TypeDef TIM1_OutputState,
                  TIM1_OutputNState_TypeDef TIM1_OutputNState,
                  uint16_t TIM1_Pulse,
                  TIM1_OCPolarity_TypeDef TIM1_OCPolarity,
                  TIM1_OCNPolarity_TypeDef TIM1_OCNPolarity,
                  TIM1_OCIdleState_TypeDef TIM1_OCIdleState,
                  TIM1_OCNIdleState_TypeDef TIM1_OCNIdleState);

void _TIM1_OC4Init(TIM1_OCMode_TypeDef TIM1_OCMode,
                  TIM1_OutputState_TypeDef TIM1_OutputState,
                  uint16_t TIM1_Pulse,
                  TIM1_OCPolarity_TypeDef TIM1_OCPolarity,
                  TIM1_OCIdleState_TypeDef TIM1_OCIdleState);

void _TIM1_SetCompare1(uint16_t Compare1);
void _TIM1_SetCompare2(uint16_t Compare2);
void _TIM1_SetCompare4(uint16_t Compare4);
uint16_t _TIM1_GetCapture1(void);
uint16_t _TIM1_GetCapture2(void);
uint16_t _TIM1_GetCapture4(void);

#endif  // _HALUTILS_H