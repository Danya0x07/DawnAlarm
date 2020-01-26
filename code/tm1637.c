#include "tm1637.h"
#include "utils.h"

#define TM_GPIO_PORT    GPIOA
#define TM_DIN_PIN  GPIO_PIN_2
#define TM_CLK_PIN  GPIO_PIN_3

#define tm_clk_0()  (TM_GPIO_PORT->ODR &= ~TM_CLK_PIN)
#define tm_clk_1()  (TM_GPIO_PORT->ODR |= TM_CLK_PIN)
#define tm_din_0()  (TM_GPIO_PORT->ODR &= ~TM_DIN_PIN)
#define tm_din_1()  (TM_GPIO_PORT->ODR |= TM_DIN_PIN)
#define tm_din_is_1()   (TM_GPIO_PORT->ODR & TM_DIN_PIN)

/*
      A
     ---
  F |   | B
     -G-
  E |   | C
     ---
      D
*/
const uint8_t tm_font[] = {
    // XGFEDCBA
    0x3f,    // 0 0b00111111
    0x06,    // 1 0b00000110
    0x5b,    // 2 0b01011011
    0x4f,    // 3 0b01001111
    0x66,    // 4 0b01100110
    0x6d,    // 5 0b01101101
    0x7d,    // 6 0b01111101
    0x07,    // 7 0b00000111
    0x7f,    // 8 0b01111111
    0x6f,    // 9 0b01101111
    0x77,    // A 0b01110111
    0x7c,    // b 0b01111100
    0x39,    // C 0b00111001
    0x5e,    // d 0b01011110
    0x79,    // E 0b01111001
    0x71,    // F 0b01110001
    0x40,    // - 0b01000000
    0x00     // пустота 0b00000000
};

enum {
    MINUS = 16,
    CLEAR,
};

static void tm1637_transmission_start(void);
static void tm1637_transmission_handle_ack(void);
static void tm1637_transmission_stop(void);
static void tm1637_write_byte(uint8_t);


void tm1637_setup(void)
{
    TM_GPIO_PORT->DDR |= TM_CLK_PIN | TM_DIN_PIN;
    TM_GPIO_PORT->ODR |= TM_CLK_PIN | TM_DIN_PIN;
}

void tm1637_display(int16_t number, bool dots)
{
    tm1637_transmission_start();
    tm1637_write_byte(0x40);  // автосдвиг курсора
    tm1637_transmission_stop();

    tm1637_transmission_start();
    tm1637_write_byte(0xC0);  // адрес 1-го сегмента
    if (number < 0) {
        tm1637_write_byte(tm_font[MINUS]);
        number = -number;
    }
    else {
        tm1637_write_byte(tm_font[number / 1000]);
    }
    number %= 1000;
    tm1637_write_byte(tm_font[number / 100] | dots << 7);
    number %= 100;
    tm1637_write_byte(tm_font[number / 10]);
    number %= 10;
    tm1637_write_byte(tm_font[number]);
    tm1637_transmission_stop();
}

void tm1637_set_displaying(bool displaying)
{
    tm1637_transmission_start();
    tm1637_write_byte(0x80 | TM_BRIGHTNESSS | ((uint8_t) displaying << 3));
    tm1637_transmission_stop();
}

static void tm1637_transmission_start(void)
{
    tm_clk_1();
    tm_din_1();
    delay_ms(1);
    tm_din_0();
}

static void tm1637_transmission_handle_ack(void)
{
    tm_clk_0();
    delay_ms(1);
    tm_din_0();
    while (tm_din_is_1());
    tm_din_1();
    tm_clk_1();
    delay_ms(1);
    tm_clk_0();
}

static void tm1637_transmission_stop(void)
{
    tm_clk_0();
    delay_ms(1);
    tm_din_0();
    delay_ms(1);
    tm_clk_1();
    delay_ms(1);
    tm_din_1();
}

static void tm1637_write_byte(uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        tm_clk_0();
        if (data & 1)
            tm_din_1();
        else
            tm_din_0();
        data >>= 1;
        delay_ms(1);
        tm_clk_1();
        delay_ms(1);
    }
    tm1637_transmission_handle_ack();
}
