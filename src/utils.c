#include "utils.h"

static volatile uint16_t time = 0;

void delay_ms(uint16_t us)
{
    time = 0;
    while (time < us);
}

void __delay_irq_handler(void) __interrupt(23)
{
    time++;
    TIM4->SR1 = (uint8_t) ~TIM4_IT_UPDATE;
}

void i2c_mem_write(uint8_t slave_addr, uint8_t *data, uint8_t count)
{
    while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY));
    I2C_GenerateSTART(ENABLE);
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(slave_addr, I2C_DIRECTION_TX);
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    while (count--) {
        I2C_SendData(*data++);
        while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
    I2C_GenerateSTOP(ENABLE);
}

void i2c_mem_read(uint8_t slave_addr, uint8_t *buffer, uint8_t count)
{
    I2C_GenerateSTART(ENABLE);
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(slave_addr, I2C_DIRECTION_RX);
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    I2C_AcknowledgeConfig(I2C_ACK_CURR);
    for (; count; count--) {
        if (count == 1) {
            I2C_AcknowledgeConfig(I2C_ACK_NONE);
        }
        while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));
        *buffer++ = I2C_ReceiveData();
    }
    I2C_GenerateSTOP(ENABLE);
}

void i2c_set_mem_ptr(uint8_t slave_addr, uint8_t *mem_addr, uint8_t addr_size)
{
    I2C_GenerateSTART(ENABLE);
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(slave_addr, I2C_DIRECTION_TX);
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    while (addr_size--) {
        I2C_SendData(*mem_addr++);
        while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
}
