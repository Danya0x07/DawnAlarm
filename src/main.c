#include "halutils.h"
#include "config.h"
#include "button.h"
#include "selector.h"
#include "eeprom.h"
#include "tm1637.h"
#include "ds1307.h"
#include "dawn.h"
#include "rgbstrip.h"

enum menu_item {
    ITEM_ALARMSETUP,
    ITEM_COLORSETUP,
    ITEM_DISKO,
    ITEM_CLOCKSETUP,
    ITEM_CANCEL,

    ITEMS_TOTAL
};

typedef volatile uint8_t binsemaphore_t;

static struct {
    binsemaphore_t update_display;
    binsemaphore_t update_dawn;
} todotable;

static bool alarm_active = TRUE;  // Используется функциями main и handle_alarm
static uint16_t current_time;
static struct options opts;

static void sys_setup(void);
static void update_time(void);
static void update_display_brightness(void);
static void handle_alarm(void);
static void perform_disko(void);

static void set_user_tape_brightness(enum color);
static enum menu_item take_user_menu_item(void);
static uint16_t take_user_time_value(bool dots);
static uint8_t take_user_dawn_duration(void);

int main(void)
{
    sys_setup();
    eeprom_load(&opts);
    dawn_setup(opts.dawn_duration);
    tm1637_set_brightness(TM_DEFAULT_BRIGHTNESS);
    current_time = ds1307_get_time();

    for (;;) {
        if (btn_pressed()) {
            RTC_SQW_OUT_GPORT->CR2 &= ~RTC_SQW_OUT_GPIN;

            if (dawn_is_started() || rgbstrip_is_active()) {
                alarm_active = FALSE;
                dawn_stop();
            } else {
                uint8_t prev_brightness = tm1637_get_brightness();

                tm1637_set_brightness(TM_DEFAULT_BRIGHTNESS);
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
                    set_user_tape_brightness(COLOR_RED);
                    set_user_tape_brightness(COLOR_GREEN);
                    set_user_tape_brightness(COLOR_BLUE);
                    break;
                case ITEM_DISKO:
                    perform_disko();
                    break;
                case ITEM_CLOCKSETUP:
                    current_time = take_user_time_value(TRUE);
                    ds1307_set_time(current_time);
                    break;
                case ITEM_CANCEL:
                    break;
                }

                tm1637_set_brightness(prev_brightness);

                // если долго копались в меню, не помешает обновить текущее время.
                current_time = ds1307_get_time();
            }

            RTC_SQW_OUT_GPORT->CR2 |= RTC_SQW_OUT_GPIN;
        }

        if (todotable.update_display) {
            update_time();
            update_display_brightness();
            handle_alarm();
            todotable.update_display = 0;
        }

        if (todotable.update_dawn) {
            dawn_update();
            todotable.update_dawn = 0;
        }
    }
}

static void sys_setup(void)
{
    // Отключение тактирование неиспользуемой периферии для энергосбережения.
    CLK->PCKENR1 &= ~(1 << CLK_PERIPHERAL_SPI);
#ifndef DEBUG
    CLK->PCKENR1 &= ~(1 << CLK_PERIPHERAL_UART1);
#endif
    CLK->PCKENR2 &= ~(1 << (CLK_PERIPHERAL_AWU & 0x0F));

    // Настройка пинов на выходы с низким лог. уровнем для энергосбережения.
#ifdef UNUSED_PINS_OF_PORTC
    GPIOC->DDR |= UNUSED_PINS_OF_PORTC;
    GPIOC->CR1 |= UNUSED_PINS_OF_PORTC;
#endif

#ifdef UNUSED_PINS_OF_PORTD
    GPIOD->DDR |= UNUSED_PINS_OF_PORTD;
    GPIOD->CR1 |= UNUSED_PINS_OF_PORTD;
#endif

    // Настройка используемых GPIO.
    TM1637_GPORT->DDR |= TM1637_CLK_GPIN | TM1637_DIN_GPIN;
    TM1637_GPORT->ODR |= TM1637_CLK_GPIN | TM1637_DIN_GPIN;
    RTC_SQW_OUT_GPORT->CR1 |= RTC_SQW_OUT_GPIN;
    RTC_SQW_OUT_GPORT->CR2 |= RTC_SQW_OUT_GPIN;
#if (DAWNALARM_MK == 2)
    BUTTON_GPORT->CR1 |= BUTTON_GPIN;  // MK1 has external pulldown.
    ENCODER_GPORT->CR1 |= ENCODER_CHA_GPIN | ENCODER_CHB_GPIN;
    BUZZER_GPORT->DDR |= BUZZER_GPIN;
    BUZZER_GPORT->CR1 |= BUZZER_GPIN;
#endif

    // Настройка внешних прерываний.
    EXTI->CR1 |= EXTI_SENSITIVITY_RISE_ONLY << 0;
    EXTI->CR1 |= EXTI_SENSITIVITY_RISE_ONLY << 6;

    // ADC1 для считывания положения потенциометра, либо, заряда аккумулятора.
    ADC1->CR1 |= ADC1_PRESSEL_FCPU_D12;
    ADC1->CR2 |= ADC1_ALIGN_RIGHT;

#if (DAWNALARM_MK == 1)
    ADC1->CSR |= POTENTIOMETER_ADC_CH;
#elif (DAWNALARM_MK == 2)
    ADC1->CSR |= BATTERY_CHARGE_ADC_CH;
#endif

    ADC1->CR1 |= ADC1_CR1_ADON;

    // TIM1 для RGB-ленты.
    TIM1_TimeBaseInit(20, TIM1_COUNTERMODE_UP, RGB_MAX_VALUE, 0);

    TIM1_OC1Init(TIM1_OCMODE_PWM1,
                 TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH,
                 TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);

    TIM1_OC2Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE,
                 TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH,
                 TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);

    TIM1_OC4Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, 0,
                 TIM1_OCPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET);

    TIM1_CtrlPWMOutputs(ENABLE);
    TIM1_Cmd(ENABLE);

    // TIM4 для функций задержки.
    TIM4->PSCR = TIM4_PRESCALER_16;
    TIM4->ARR = 0xFF;
    TIM4->CR1 |= TIM4_CR1_CEN;

    // TIM2 для процедуры рассвета.
    TIM2->PSCR = TIM2_PRESCALER_32;  // прерывание 4 раза в секунду
    TIM2->ARRH = (uint8_t)(15624 >> 8);
    TIM2->ARRL = (uint8_t)15624;
    TIM2->SR1 = (uint8_t) ~TIM2_FLAG_UPDATE;
    TIM2->IER |= TIM2_IT_UPDATE;

    // I2C для часов реального времени и микросхемы EEPROM.
    I2C_Init(I2C_MAX_STANDARD_FREQ, 0x54, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 2);
    I2C_Cmd(ENABLE);

    enableInterrupts();
}

INTERRUPT_HANDLER(clock_dots_irq, ITC_IRQ_PORTC)
{
    RTC_SQW_OUT_GPORT->CR2 &= ~RTC_SQW_OUT_GPIN;
    delay_ms(1);
    if (!(RTC_SQW_OUT_GPORT->IDR & RTC_SQW_OUT_GPIN))
        goto out;
    todotable.update_display = 1;
out:
    RTC_SQW_OUT_GPORT->CR2 |= RTC_SQW_OUT_GPIN;
}

INTERRUPT_HANDLER(dawn_increase_irg, 13)
{
    todotable.update_dawn = 1;
    TIM2->SR1 = (uint8_t) ~TIM2_IT_UPDATE;
}

static void update_time(void)
{
    static uint8_t counter = 0;
    static bool dots = FALSE;

    if (++counter >> 1 > 10) {  // Раз в 10 секунд обновляем время.
        current_time = ds1307_get_time();
        counter = 0;
    }
    tm1637_display_dec(current_time, dots = !dots);
}

static void update_display_brightness(void)
{
    static bool prev_night = 0;
    bool night = ((current_time >= 2000 && current_time < 2400) ||
                  (current_time < 600));
    night &= !rgbstrip_is_active();

    if (night != prev_night){
        if (night)
            tm1637_set_brightness(TM_NIGHT_BRIGHTNESS);
        else
            tm1637_set_brightness(TM_DEFAULT_BRIGHTNESS);
        prev_night = night;
    }
}

static void handle_alarm(void)
{
    // Рассчётное время полного рассвета относительно текущего времени.
    uint16_t estimated_alarm = current_time + opts.dawn_duration;
    bool not_too_late = current_time <= opts.alarm_time;

    // Коррекция рассчётного времени под временной формат.
    if (estimated_alarm % 100 > 59)
        estimated_alarm += 40;
    if (estimated_alarm / 100 > 23)
        estimated_alarm -= 2400;
    /*
     * Самая неказистая ситуация: будильник на 00:ХХ, где
     * ХХ < длительности рассвета, при том, если текущее время где-то между
     * 23:45 -- 00:00, то стандартная проверка ломается. Чтобы этого избежать,
     * пришлось переинтерпретировать условие "ещё не поздно".
     */
    if (opts.alarm_time < opts.dawn_duration)
        not_too_late = opts.alarm_time + opts.dawn_duration > estimated_alarm;

    if (estimated_alarm >= opts.alarm_time && not_too_late) {
        if (alarm_active && !dawn_is_started())
            dawn_start();
    } else {
        alarm_active = TRUE;
    }
}

static void perform_disko(void)
{
    enum color incr_color = COLOR_GREEN;
    enum color decr_color = COLOR_RED;
    uint8_t smiley[4] = {0x18, 0xEB, 0x6B, 0x0C};
    uint16_t i;

    tm1637_display_content(smiley);
    while (btn_is_pressed());
    delay_ms(10);  // кнопочный дребезг

    while (!btn_is_pressed()) {
        for (i = 0; i < RGB_MAX_VALUE + 1 && !btn_is_pressed(); i++) {
            rgbstrip_set(incr_color, i);
            rgbstrip_set(decr_color, RGB_MAX_VALUE - i);
            delay_ms(20);
        }
        decr_color = incr_color;
        incr_color = rgbstrip_change_color(incr_color);
    }
    rgbstrip_set_R(0);
    rgbstrip_set_G(0);
    rgbstrip_set_B(0);
}

static void set_user_tape_brightness(enum color c)
{
    uint8_t val = 0, _val = 0;
    uint8_t disp_content[4] = {TM_CLEAR, TM_0, TM_0, TM_0};

    selector_set(0, RGB_MAX_VALUE, 0);
    selector_irq_on();
    while (!btn_pressed()) {
        val = selector_get();
        if (val != _val) {
            disp_content[1] = tm_digits[val / 100];
            disp_content[2] = tm_digits[(val / 10) % 10];
            disp_content[3] = tm_digits[val % 10];
            tm1637_display_content(disp_content);
            rgbstrip_set(c, val);
            _val = val;
        }
        delay_ms(10);
    }
    selector_irq_off();
}

static enum menu_item take_user_menu_item(void)
{
    enum menuprev_item item = 0, prev_item = (enum menuprev_item)0xFF;
    uint8_t menu[ITEMS_TOTAL][4] = {
        {TM_A, TM_L, TM_A, TM_r},
        {TM_CLEAR, TM_C, TM_0, TM_L},
        {TM_d, TM_I, TM_C, TM_0},
        {TM_C, TM_L, TM_0, TM_C},
        {TM_0, TM_E, TM_H, TM_A}
    };

    selector_set(0, ITEMS_TOTAL - 1, ITEM_ALARMSETUP);
    selector_irq_on();
    while (!btn_pressed()) {
        item = selector_get();
        if (item != prev_item) {
            tm1637_display_content(menu[item]);
            prev_item = item;
        }
        delay_ms(10);
    }
    selector_irq_off();
    return item;
}

static uint16_t take_user_time_value(bool dots)
{
    uint8_t hours = 0, minutes = 0;
    uint8_t _hours = 0xFF, _minutes = 0xFF;

    selector_set(0, 23, current_time / 100);
    selector_irq_on();
    while (!btn_pressed()) {
        hours = selector_get();
        if (hours != _hours) {
            tm1637_display_dec(hours * 100 + minutes, dots);
            _hours = hours;
        }
        delay_ms(10);
    }

    selector_set(0, 59, current_time % 100);
    while (!btn_pressed()) {
        minutes = selector_get();
        if (minutes != _minutes) {
            tm1637_display_dec(hours * 100 + minutes, dots);
            _minutes = minutes;
        }
        delay_ms(10);
    }
    selector_irq_off();
    return hours * 100 + minutes;
}

static uint8_t take_user_dawn_duration(void)
{
    uint8_t dawn_duration = 0, _dawn_duration = 0xFF;
    uint8_t disp_content[4] = {TM_d, TM_d, TM_0, TM_0};

    selector_set(5, 20, 20);
    selector_irq_on();
    while (!btn_pressed()) {
        dawn_duration = selector_get();
        if (dawn_duration != _dawn_duration) {
            disp_content[2] = tm_digits[dawn_duration / 10];
            disp_content[3] = tm_digits[dawn_duration % 10];
            tm1637_display_content(disp_content);
            _dawn_duration = dawn_duration;
        }
        delay_ms(10);
    }
    selector_irq_off();
    return dawn_duration;
}