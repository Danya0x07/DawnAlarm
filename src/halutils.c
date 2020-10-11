#include "halutils.h"

void delay_ms(uint16_t ms)
{
    while (ms--) {
        // задержка на 1ms
        tim4_set_counter(0);
        while (tim4_get_counter() < 125)
            ;
    }
}

static void i2c_start(uint8_t addr, uint8_t direction)
{
    I2C_GenerateSTART(ENABLE);
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(addr, direction);
    if (direction == 0)
        while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    else
        while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
}

static void i2c_stop(void)
{
    I2C_GenerateSTOP(ENABLE);
}

static void i2c_write(const uint8_t *data, uint8_t len)
{
    while (len--) {
        I2C_SendData(*data++);
        while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
}

static void i2c_read(uint8_t *data, uint8_t len)
{
    I2C_AcknowledgeConfig(I2C_ACK_CURR);
    for (; len; len--) {
        if (len == 1) {
            I2C_AcknowledgeConfig(I2C_ACK_NONE);
        }
        while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));
        *data++ = I2C_ReceiveData();
    }
}

void i2c_write_bytes(uint8_t addr, const uint8_t *data, uint8_t len, 
                     uint8_t flags)
{
    if (!(flags & I2C_NOSTART))
        i2c_start(addr, I2C_DIRECTION_TX);
    i2c_write(data, len);
    if (!(flags & I2C_NOSTOP))
        i2c_stop();
}

void i2c_read_bytes(uint8_t addr, uint8_t *data, uint8_t len, 
                    uint8_t flags)
{
    if (!(flags & I2C_NOSTART))
        i2c_start(addr, I2C_DIRECTION_RX);
    i2c_read(data, len);
    if (!(flags & I2C_NOSTOP))
        i2c_stop();
}
