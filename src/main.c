/* Copyright (C) 2020 Daniel Efimenko
 *     github.com/Danya0x07
 *------------------------------------
 * Программное обеспечение для будильника рассвета DawnAlarm mkI и mkII.
 */

#include "halutils.h"
#include "config.h"
#include "button.h"
#include "selector.h"
#include "tm1637.h"
#include "rtc.h"
#include "dawn.h"
#include "rgbstrip.h"
#include "ui.h"
#include "buzzer.h"
#include "battery.h"

static struct settings {
    uint16_t alarm_time;     // время полного рассвета
    uint8_t  dawn_duration;  // длительность рассвета (в минутах)
    bool alarm_enabled;      // включён ли вообще будильник или работаем просто как часы
#if (DAWNALARM_MK == 2)
    bool buzzer_enabled;     // пищать ли буззером при окончании рассвета
#endif
} device_settings;

/* Самая минималистичная реализация чего-то вроде планировщика задач.
 * В прерываниях выставляем TODO-флаги, а потом в суперцикле обрабатываем дела.
 */
typedef volatile uint8_t todothing_t;

static struct {
    todothing_t update_display;
    todothing_t update_dawn;
    todothing_t handle_btn_press;
} todotable;

static int16_t current_time;
static bool dawn_performed;
static bool buzz_performed;

static void sys_setup(void);
static void handle_button_press(void);
static void update_time_and_display(void);
static void update_display_brightness(int16_t current_time);

int main(void)
{
    sys_setup();
    rtc_access_nvram((uint8_t *)&device_settings, sizeof(struct settings), RTC_NVRAM_LOAD);
    dawn_setup(device_settings.alarm_time, device_settings.dawn_duration);
    
    if (!rtc_is_running()) {  // Если это первое включение, настраиваем время,
        tm1637_set_brightness(7);
        rtc_setup(ui_get_user_time(0, TRUE));
    } else {  // иначе показываем брутальный экран приветствия.
        ui_show_splash_screen();
        tm1637_set_brightness(7);
        delay_ms(700);
    }

#if (DAWNALARM_MK == 2)
    if (battery_level_is_low()) {
        ui_show_battery_level_low();
    }
#endif

    current_time = rtc_get_time();
    rtc_irq_on();
    button_irq_on();

    for (;;) {
        if (todotable.handle_btn_press) {
            handle_button_press();
            todotable.handle_btn_press = 0;
        }

        if (todotable.update_display) {
            update_time_and_display();
            todotable.update_display = 0;
        }

        if (todotable.update_dawn) {
            dawn_update(current_time);
            todotable.update_dawn = 0;
        }

        wfi();
    }
}

INTERRUPT_HANDLER(button_irq, ITC_IRQ_PORTA)
{
    todotable.handle_btn_press = 1;
}

INTERRUPT_HANDLER(rtc_sqw_irq, ITC_IRQ_PORTC)
{
    todotable.update_display = 1;
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
#if (DAWNALARM_MK == 2)
    BUTTON_GPORT->CR1 |= BUTTON_GPIN;  // MK1 has external pulldown.
    ENCODER_GPORT->CR1 |= ENCODER_CHA_GPIN | ENCODER_CHB_GPIN;
    BUZZER_GPORT->DDR |= BUZZER_GPIN;
    BUZZER_GPORT->CR1 |= BUZZER_GPIN;
#endif

    // Настройка внешних прерываний.
    EXTI->CR1 |= EXTI_SENSITIVITY_FALL_ONLY << 0;  // для кнопки
    EXTI->CR1 |= EXTI_SENSITIVITY_RISE_ONLY << 4;  // для RTC
#if (DAWNALARM_MK == 2)
    EXTI->CR1 |= EXTI_SENSITIVITY_RISE_ONLY << 6;  // для энкодера
#endif

    // ADC1 для считывания положения потенциометра, либо, заряда аккумулятора.
    ADC1->CR1 |= ADC1_PRESSEL_FCPU_D18;
    ADC1->CR2 |= ADC1_ALIGN_RIGHT;

#if (DAWNALARM_MK == 1)
    ADC1->CSR |= POTENTIOMETER_ADC_CH;
    ADC1->TDRL = 1 << POTENTIOMETER_ADC_CH;
#elif (DAWNALARM_MK == 2)
    ADC1->CSR |= BATTERY_ADC_CH;
    ADC1->TDRL = 1 << BATTERY_ADC_CH;
#endif

    ADC1->CR1 |= ADC1_CR1_ADON;

    // TIM1 для RGB-ленты.
    _TIM1_TimeBaseInit(100, TIM1_COUNTERMODE_UP, RGB_MAX_VALUE, 0);

    _TIM1_OC1Init(TIM1_OCMODE_PWM1,
                 TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH,
                 TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);

    _TIM1_OC2Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE,
                 TIM1_OUTPUTNSTATE_DISABLE,
                 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH,
                 TIM1_OCIDLESTATE_RESET, TIM1_OCNIDLESTATE_RESET);

    _TIM1_OC4Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, 0,
                 TIM1_OCPOLARITY_HIGH, TIM1_OCIDLESTATE_RESET);

    TIM1->BKR |= TIM1_BKR_MOE;
    TIM1->CR1 |= TIM1_CR1_CEN;

    // TIM4 для функций задержки.
    TIM4->PSCR = TIM4_PRESCALER_16;
    TIM4->ARR = 0xFF;
    TIM4->CR1 |= TIM4_CR1_CEN;

    // I2C для часов реального времени.
    _I2C_Init(I2C_MAX_STANDARD_FREQ, 0x54, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 2);
    I2C->CR1 |= I2C_CR1_PE;

    enableInterrupts();
}

static void handle_button_press(void)
{
    button_irq_off();
    rtc_irq_off();

    if (!buzz_performed && buzzer_is_on()) {
        buzzer_off();
        buzz_performed = TRUE;
    }
    else if (device_settings.alarm_enabled && !dawn_performed && 
                dawn_is_ongoing(current_time)) {
        buzz_performed = dawn_performed = TRUE;
        todotable.update_dawn = 0;
        rgbstrip_kill();
    } 
    else if (rgbstrip_is_active()) {
        rgbstrip_kill();
    }
    else {
        bool settings_changed = FALSE;
        uint8_t prev_brightness = tm1637_get_brightness();

        tm1637_set_brightness(7);
        switch (ui_get_user_menu_item())
        {
        case ITEM_ALARMSETUP:
            device_settings.alarm_enabled = ui_get_user_boolean();
            if (device_settings.alarm_enabled) {
                device_settings.alarm_time = ui_get_user_time(current_time, FALSE);
                device_settings.dawn_duration = ui_get_user_dawn_duration();
#if (DAWNALARM_MK == 2)
                device_settings.buzzer_enabled = ui_get_user_buzzer_status();
                if (device_settings.buzzer_enabled)
                    buzz_performed = FALSE;
#endif
                dawn_setup(device_settings.alarm_time, device_settings.dawn_duration);
                dawn_performed = FALSE;
            }
            settings_changed = TRUE;
            break;
#if (DAWNALARM_MK == 2)
        case ITEM_BUZZERSETUP:
            device_settings.buzzer_enabled = ui_get_user_boolean();
            if (device_settings.buzzer_enabled) {
                buzzer_buzz(1, 200);
                buzz_performed = FALSE;
            }
            settings_changed = TRUE;
            break;

        case ITEM_SHOWCONF:
            if (device_settings.alarm_enabled) {
                ui_show_configuration(device_settings.alarm_time, 
                                      device_settings.dawn_duration, 
                                      device_settings.buzzer_enabled);
            }
            else {
                ui_show_thereis_noconf();
            }
            break;
#endif
        case ITEM_COLORSETUP:
            ui_set_strip_colors_brightness();
            break;
        case ITEM_DISKO:
            ui_perform_disko();
            break;
        case ITEM_CLOCKSETUP:
            current_time = ui_get_user_time(current_time, TRUE);
            rtc_set_time(current_time);
            break;
#if (DAWNALARM_MK == 2)
        case ITEM_CHARGE:
            ui_show_charge_level();
            break;
#endif
        case ITEM_CANCEL:
            break;
        }
        tm1637_set_brightness(prev_brightness);
        if (settings_changed)
            rtc_access_nvram((uint8_t *)&device_settings, sizeof(struct settings), RTC_NVRAM_SAVE);
        // если долго копались в меню, не помешает обновить текущее время.
        current_time = rtc_get_time();
        update_display_brightness(current_time);
    }
    rtc_irq_on();
    button_irq_on();
}

static void update_display_brightness(int16_t current_time)
{
    static bool prev_darktime = FALSE;
    bool darktime = rgbstrip_is_active() == FALSE;

    darktime &= ((current_time >= 2000 && current_time < 2400) ||
                 (current_time < 600));
    if (darktime != prev_darktime) {
        tm1637_set_brightness(7 * !darktime);
        prev_darktime = darktime;
    }
}

static inline void handle_day_transition(int16_t current_time)
{
    static int16_t prev_time = 0;

    if (current_time < prev_time) {  // наступил следующий день
        buzz_performed = dawn_performed = FALSE;
    }
    prev_time = current_time;
}

static void update_time_and_display(void)
{
    static uint8_t counter = 0;
    static bool dots = FALSE;
    
    update_display_brightness(current_time);
    if (++counter > 10) {  // Раз в 5 секунд обновляем состояние логики будильника.
        current_time = rtc_get_time();
        handle_day_transition(current_time);
        if (device_settings.alarm_enabled && !dawn_performed) {
            todotable.update_dawn = 1;
#if (DAWNALARM_MK == 2)
            if (device_settings.buzzer_enabled && 
                    current_time == device_settings.alarm_time &&
                    !buzzer_is_on() && !buzz_performed)
                buzzer_on();
#endif
        }
        counter = 0;
    }
    tm1637_display_dec(current_time, dots = !dots);
}