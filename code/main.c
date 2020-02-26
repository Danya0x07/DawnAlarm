#include <stm8s.h>
#include "utils.h"
#include "input.h"
#include "eeprom.h"
#include "tm1637.h"
#include "ds1307.h"
#include "dawn.h"
#include "rgbtape.h"

enum menu_item {
    ITEM_ALARMSETUP,
    ITEM_COLORSETUP,
    ITEM_DISKO,
    ITEM_CLOCKSETUP,
    ITEM_CANCEL,

    ITEMS_TOTAL
};

static bool alarm_active = TRUE;  // Используется функциями main и handle_alarm

static void sys_setup(void);
static void update_time(uint16_t *current_time);
static void handle_alarm(const struct options *, uint16_t current_time);
static void perform_disko(void);

static void set_user_brightness(enum color);
static enum menu_item take_user_menu_item(void);
static uint16_t take_user_time_value(bool dots);
static uint8_t take_user_dawn_duration(void);

int main(void)
{
    static struct options opts;
    uint16_t current_time;

    sys_setup();
    input_setup();
    tm1637_gpio_setup();
    tm1637_set_displaying(TRUE);
    eeprom_load(&opts);
    dawn_setup(opts.dawn_duration);
    current_time = ds1307_get_time();

    while (1) {
        if (btn_pressed()) {
            if (btn_pressed_again()) {  // Двукратное нажатие - вход в меню.
                switch (take_user_menu_item())
                {
                case ITEM_ALARMSETUP:
                    opts.alarm_time = take_user_time_value(FALSE);
                    opts.dawn_duration = take_user_dawn_duration();
                    dawn_setup(opts.dawn_duration);
                    eeprom_save(&opts);
                    alarm_active = TRUE;
                    break;
                case ITEM_COLORSETUP:
                    set_user_brightness(COLOR_RED);
                    set_user_brightness(COLOR_GREEN);
                    set_user_brightness(COLOR_BLUE);
                    break;
                case ITEM_DISKO:
                    perform_disko();
                    break;
                case ITEM_CLOCKSETUP:
                    current_time = take_user_time_value(TRUE);
                    ds1307_setup(current_time);
                    break;
                case ITEM_CANCEL:
                    break;
                }
                // если долго копались в меню, не помешает обновить текущее время.
                current_time = ds1307_get_time();
            } else {  // Однократное нажатие - или ничего, или выкл светодиоды.
                if (dawn_is_started() || rgbtape_is_active()) {
                    alarm_active = FALSE;
                    dawn_stop();
                }
            }
        }

        update_time(&current_time);
        handle_alarm(&opts, current_time);
    }
}

static void sys_setup(void)
{
    // Отключение тактирование неиспользуемой периферии для энергосбережения.
    CLK->PCKENR1 &= ~(1 << CLK_PERIPHERAL_SPI);
    CLK->PCKENR1 &= ~(1 << CLK_PERIPHERAL_UART1);
    CLK->PCKENR2 &= ~(1 << (CLK_PERIPHERAL_AWU & 0x0F));
    // Настройка пинов на выходы с низким лог. уровнем для энергосбережения.
    GPIOC->DDR |= GPIO_PIN_5;
    GPIOC->CR1 |= GPIO_PIN_5;
    GPIOD->DDR |= GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    GPIOD->CR1 |= GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    // ADC1 для считывания положения потенциометра
    ADC1->CR1 |= ADC1_PRESSEL_FCPU_D6;
    ADC1->CR2 |= ADC1_ALIGN_RIGHT;
    ADC1->CSR |= ADC1_CHANNEL_4;
    ADC1->CR1 |= ADC1_CR1_ADON;
    // TIM1 для RGB-ленты
    TIM1_TimeBaseInit(20, TIM1_COUNTERMODE_UP, RGB_MAX_VALUE, 0);
    TIM1_OC1Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);
    TIM1_OC2Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);
    TIM1_OC4Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, 0, TIM1_OCPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET);
    TIM1_CtrlPWMOutputs(ENABLE);
    TIM1_Cmd(ENABLE);
    // TIM4 для функций задержки
    TIM4->PSCR = TIM4_PRESCALER_16;
    TIM4->ARR = 124;
    TIM4->SR1 = (uint8_t) ~TIM4_FLAG_UPDATE;
    TIM4->IER |= TIM4_IT_UPDATE;
    TIM4->CR1 |= TIM4_CR1_CEN;
    // TIM2 для процедуры рассвета
    TIM2->PSCR = TIM2_PRESCALER_32;  // прерывание 4 раза в секунду
    TIM2->ARRH = (uint8_t)(15624 >> 8);
    TIM2->ARRL = (uint8_t) 15624;
    TIM2->SR1 = (uint8_t) ~TIM2_FLAG_UPDATE;
    TIM2->IER |= TIM2_IT_UPDATE;
    // I2C для часов реального времени и микросхемы EEPROM
    I2C_Init(I2C_MAX_STANDARD_FREQ, 0x54, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 2);
    I2C_Cmd(ENABLE);

    enableInterrupts();
}

static void update_time(uint16_t *current_time)
{
    static uint8_t pulse_counter = 0;
    static bool dots = FALSE, _sq_state = FALSE;

    bool sq_state = ds1307_sqwout_is_1();
    if (sq_state != _sq_state) {
        pulse_counter++;
        // Состояние пина SQW/OUT меняется 2 раза в секунду,
        if (pulse_counter % 2 == 0)  // а мы переключаем двоеточие раз в секунду.
            tm1637_display_dec(*current_time, dots = !dots);
        if (pulse_counter / 2 > 10) {  // Раз в 10 секунд обновляем время.
            *current_time = ds1307_get_time();
            pulse_counter = 0;
        }
        _sq_state = sq_state;
    }
}

static void handle_alarm(const struct options *opts, uint16_t current_time)
{
    // Рассчётное время полного рассвета относительно текущего времени.
    uint16_t estimated_alarm = current_time + opts->dawn_duration;
    bool not_too_late = current_time <= opts->alarm_time;
    // Коррекция рассчётного времени под временной формат.
    if (estimated_alarm % 100 > 59)
        estimated_alarm += 40;
    if (estimated_alarm / 100 > 23)
        estimated_alarm -= 2400;
    /* Самая неказистая ситуация: будильник на 00:ХХ, где ХХ < длительности рассвета,
     * при том, если текущее время где-то между 23:45 -- 00:00,
     * то стандартная проверка ломается. Чтобы этого избежать, пришлось
     * переинтерпретировать условие "ещё не поздно". */
    if (opts->alarm_time < opts->dawn_duration)
        not_too_late = opts->alarm_time + opts->dawn_duration > estimated_alarm;

    if (estimated_alarm >= opts->alarm_time && not_too_late) {
        if (alarm_active && !dawn_is_started())
            dawn_start();
    } else {
        alarm_active = TRUE;
    }
}

static void perform_disko(void)
{
    enum color incr_color = COLOR_BLUE;
    enum color decr_color = COLOR_RED;
    uint16_t i;
    while (!btn_pressed()) {
        for (i = 0; i < RGB_MAX_VALUE + 1; i++) {
            rgbtape_set(incr_color, i);
            rgbtape_set(decr_color, RGB_MAX_VALUE - i);
            delay_ms(20);
        }
        decr_color = incr_color;
        incr_color = rgbtape_change_color(incr_color);
    }
    rgbtape_set_R(0);
    rgbtape_set_G(0);
    rgbtape_set_B(0);
}

static void set_user_brightness(enum color c)
{
    uint8_t val = 0, _val = 0;
    enum tm_charset disp_content[4] = {TM_CLEAR, 0, 0, 0};
    while (!btn_pressed()) {
        val = potentiometer_get(RGB_MAX_VALUE + 1);
        if (val != _val) {
            disp_content[1] = val / 100;
            disp_content[2] = (val / 10) % 10;
            disp_content[3] = val % 10;
            tm1637_display_chars(disp_content, FALSE);
            rgbtape_set(c, val);
            _val = val;
        }
        delay_ms(10);
    }
}

static enum menu_item take_user_menu_item(void)
{
    enum menu_item item = 0, _item = 0;
    enum tm_charset menu[ITEMS_TOTAL][4] = {
        {TM_A, TM_L, TM_A, TM_r},
        {TM_CLEAR, TM_C, TM_0, TM_L},
        {TM_d, TM_I, TM_C, TM_0},
        {TM_C, TM_L, TM_0, TM_C},
        {TM_0, TM_E, TM_H, TM_A}
    };

    while (!btn_pressed()) {
        item = potentiometer_get(ITEMS_TOTAL);
        if (item != _item) {
            tm1637_display_chars(menu[item], FALSE);
            _item = item;
        }
    }
    return item;
}

static uint16_t take_user_time_value(bool dots)
{
    uint8_t hours = 0, minutes = 0;
    uint8_t _hours = 0, _minutes = 0;
    while (!btn_pressed()) {
        hours = potentiometer_get(24);
        if (hours != _hours) {
            tm1637_display_dec(hours * 100 + minutes, dots);
            _hours = hours;
        }
        delay_ms(10);
    }
    while (!btn_pressed()) {
        minutes = potentiometer_get(60);
        if (minutes != _minutes) {
            tm1637_display_dec(hours * 100 + minutes, dots);
            _minutes = minutes;
        }
        delay_ms(10);
    }
    return hours * 100 + minutes;
}

static uint8_t take_user_dawn_duration(void)
{
    uint8_t dawn_duration = 0, _dawn_duration = 0;
    enum tm_charset disp_content[4] = {TM_d, TM_d, 0, 0};
    while (!btn_pressed()) {
        dawn_duration = potentiometer_get(16);
        if (dawn_duration != _dawn_duration) {
            disp_content[2] = dawn_duration / 10;
            disp_content[3] = dawn_duration % 10;
            tm1637_display_chars(disp_content, FALSE);
            _dawn_duration = dawn_duration;
        }
        delay_ms(10);
    }
    return dawn_duration;
}
