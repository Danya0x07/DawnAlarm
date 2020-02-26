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
    0b01110110,  // H
    0b00110000,  // I
    0b00111000,  // L
    0b01110011,  // P
    0b01100111,  // q
    0b00110001,  // r
    0b00111110,  // U
    0b01000000,  // -
    0b00001000,  // _
    0b01100011,  // градус
    0b00000000,  // пустота
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

void tm1637_gpio_setup(void)
{
    TM_GPIO_PORT->DDR |= TM_CLK_PIN | TM_DIN_PIN;
    TM_GPIO_PORT->ODR |= TM_CLK_PIN | TM_DIN_PIN;
}

void tm1637_display_dec(int16_t number, bool dots)
{
    uint8_t sequence[5] = {0};
    if (number > 9999)
        number = 9999;
    else if (number < -999)
        number = -999;
    sequence[0] = 0xC0;  // адрес 1-го сегмента
    if (number < 0) {
        sequence[1] = tm_font[TM_MINUS];
        number = -number;
    }
    else {
        sequence[1] = tm_font[number / 1000];
    }
    sequence[2] = tm_font[(number %= 1000) / 100] | dots << 7;
    sequence[3] = tm_font[(number %= 100) / 10];
    sequence[4] = tm_font[(number %= 10)];
    tm1637_send_command(0x40);  // автосдвиг курсора
    tm1637_send_sequence(sequence, sizeof(sequence));
}

void tm1637_display_chars(const enum tm_charset ch[4], bool dots)
{
    uint8_t i;
    tm1637_send_command(0x40);  // автосдвиг курсора
    tm1637_transmission_start();
    tm1637_write_byte(0xC0);  // адрес 1-го сегмента
    for (i = 0; i < 4; i++)
        tm1637_write_byte(tm_font[ch[i]] | dots << 7);
    tm1637_transmission_stop();
}

void tm1637_display_custom(uint8_t disp_content[4])
{
    uint8_t i;
    tm1637_send_command(0x40);  // автосдвиг курсора
    tm1637_transmission_start();
    tm1637_write_byte(0xC0);  // адрес 1-го сегмента
    for (i = 0; i < 4; i++)
        tm1637_write_byte(disp_content[i]);
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

static void tm1637_transmission_start(void)
{
    tm_clk_1();
    tm_din_1();
    delay_ms(1);
    tm_din_0();
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

static inline void tm1637_transmission_handle_ack(void)
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
