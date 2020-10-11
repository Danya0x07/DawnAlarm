#include "tm1637.h"
#include "config.h"

#define _clk_low()  (TM1637_GPORT->ODR &= ~TM1637_CLK_GPIN)
#define _clk_high() (TM1637_GPORT->ODR |= TM1637_CLK_GPIN)
#define _din_low()  (TM1637_GPORT->ODR &= ~TM1637_DIN_GPIN)
#define _din_high() (TM1637_GPORT->ODR |= TM1637_DIN_GPIN)
#define _din_is_high()  (TM1637_GPORT->ODR & TM1637_DIN_GPIN)

/*
      A
     ---
  F |   | B
     -G-
  E |   | C
     ---
      D
*/
const uint8_t tm_digits[0x10] = {
//    XGFEDCBA
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111,  // 9
    0b01110111,  // A
    0b01111100,  // b
    0b00111001,  // C
    0b01011110,  // d
    0b01111001,  // E
    0b01110001,  // F
};

static uint8_t current_brightness = TM_DEFAULT_BRIGHTNESS;

/* FIXME: Поскольку наш МК настроен на 2 МГц, микросекундная задержка
 * оказалась слишком запарна для реализации, и было решено забить
 * на тайминги в 2-3 мкс и заменить их на задежрку в 1 мс. Костыльно, но
 * в нашем случае не критично. При повторном использовании этой библиотеки
 * стоит заменить задержки на микросекундные, если необходимо. */
static void tm1637_send_command(uint8_t);
static void tm1637_send_sequence(const uint8_t [], uint8_t count);
static void tm1637_write_byte(uint8_t);
static void tm1637_transmission_start(void);
static void tm1637_transmission_stop(void);
static inline void tm1637_transmission_handle_ack(void);

void tm1637_display_dec(int16_t number, bool dots)
{
    uint8_t sequence[5] = {0};

    if (number > 9999)
        number = 9999;
    else if (number < -999)
        number = -999;
    sequence[0] = 0xC0;  // адрес 1-го сегмента
    if (number < 0) {
        sequence[1] = TM_MINUS;
        number = -number;
    }
    else {
        sequence[1] = tm_digits[number / 1000];
    }
    sequence[2] = tm_digits[(number %= 1000) / 100] | dots << 7;
    sequence[3] = tm_digits[(number %= 100) / 10];
    sequence[4] = tm_digits[(number %= 10)];
    tm1637_send_command(0x40);  // автосдвиг курсора
    tm1637_send_sequence(sequence, sizeof(sequence));
}

void tm1637_display_content(uint8_t content[4])
{
    uint8_t i;

    tm1637_send_command(0x40);  // автосдвиг курсора
    tm1637_transmission_start();
    tm1637_write_byte(0xC0);  // адрес 1-го сегмента
    for (i = 0; i < 4; i++)
        tm1637_write_byte(content[i]);
    tm1637_transmission_stop();
}

void tm1637_set_displaying(bool displaying)
{
    tm1637_send_command(0x80 | current_brightness | ((uint8_t)displaying << 3));
}

void tm1637_set_brightness(uint8_t brightness)
{
    current_brightness = brightness & 0x07;
    tm1637_send_command(0x80 | current_brightness | (1 << 3));
}

uint8_t tm1637_get_brightness(void)
{
    return current_brightness;
}

static void tm1637_send_command(uint8_t command)
{
    tm1637_transmission_start();
    tm1637_write_byte(command);
    tm1637_transmission_stop();
}

static void tm1637_send_sequence(const uint8_t sequence[], uint8_t count)
{
    uint8_t i;

    tm1637_transmission_start();
    for (i = 0; i < count; i++) {
        tm1637_write_byte(sequence[i]);
    }
    tm1637_transmission_stop();
}

static void tm1637_write_byte(uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        _clk_low();
        if (data & 1)
            _din_high();
        else
            _din_low();
        data >>= 1;
        delay_ms(1);
        _clk_high();
        delay_ms(1);
    }
    tm1637_transmission_handle_ack();
}

static void tm1637_transmission_start(void)
{
    _clk_high();
    _din_high();
    delay_ms(1);
    _din_low();
}

static void tm1637_transmission_stop(void)
{
    _clk_low();
    delay_ms(1);
    _din_low();
    delay_ms(1);
    _clk_high();
    delay_ms(1);
    _din_high();
}

static inline void tm1637_transmission_handle_ack(void)
{
    _clk_low();
    delay_ms(1);
    _din_low();
    while (_din_is_high());
    _din_high();
    _clk_high();
    delay_ms(1);
    _clk_low();
}
