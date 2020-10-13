#include "halutils.h"
#include "config.h"
#include "button.h"
#include "selector.h"
#include "eeprom.h"
#include "tm1637.h"
#include "ds1307.h"
#include "dawn.h"
#include "rgbstrip.h"
#include "ui.h"

typedef volatile uint8_t todothing_t;

static struct {
    todothing_t update_display;
    todothing_t update_dawn;
} todotable;

static bool alarm_active = TRUE;  // Используется функциями main и handle_alarm
static uint16_t current_time;
static struct options opts;

static void sys_setup(void);
static void update_time_and_display(void);
static void handle_alarm(void);

int main(void)
{
    sys_setup();
    ui_show_splash_screen();
    tm1637_set_brightness(TM_DEFAULT_BRIGHTNESS);
    delay_ms(1000);
    eeprom_load(&opts);
    dawn_setup(opts.dawn_duration);
    current_time = ds1307_get_time();

    for (;;) {
        if (button_pressed()) {
            rtc_irq_off();

            if (dawn_is_started() || rgbstrip_is_active()) {
                alarm_active = FALSE;
                dawn_stop();
            } else {
                uint8_t prev_brightness = tm1637_get_brightness();

                tm1637_set_brightness(TM_DEFAULT_BRIGHTNESS);
                switch (ui_get_user_menu_item())
                {
                case ITEM_ALARMSETUP:
                    opts.alarm_time = ui_get_user_time(current_time, FALSE);
                    opts.dawn_duration = ui_get_user_dawn_duration();
                    dawn_setup(opts.dawn_duration);
                    eeprom_save(&opts);
                    alarm_active = TRUE;
                    break;
                case ITEM_COLORSETUP:
                    ui_set_strip_brightness(COLOR_RED);
                    ui_set_strip_brightness(COLOR_GREEN);
                    ui_set_strip_brightness(COLOR_BLUE);
                    break;
                case ITEM_DISKO:
                    ui_perform_disko();
                    break;
                case ITEM_CLOCKSETUP:
                    current_time = ui_get_user_time(current_time, TRUE);
                    ds1307_set_time(current_time);
                    break;
                case ITEM_CANCEL:
                    break;
                }
                tm1637_set_brightness(prev_brightness);

                // если долго копались в меню, не помешает обновить текущее время.
                current_time = ds1307_get_time();
            }
            rtc_irq_on();
        }

        if (todotable.update_display) {
            update_time_and_display();
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
    todotable.update_display = 1;
}

INTERRUPT_HANDLER(dawn_increase_irg, 13)
{
    todotable.update_dawn = 1;
    TIM2->SR1 = (uint8_t) ~TIM2_IT_UPDATE;
}

static void update_time_and_display(void)
{
    static uint8_t counter = 0;
    static bool dots = FALSE;

    if (++counter >> 1 > 10) {  // Раз в 10 секунд обновляем время.
        static bool prev_darktime = FALSE;
        bool darktime = rgbstrip_is_active() == FALSE;
        current_time = ds1307_get_time();
        darktime &= ((current_time >= 2000 && current_time < 2400) ||
                     (current_time < 600));
        if (darktime != prev_darktime) {
            tm1637_set_brightness(darktime ? TM_NIGHT_BRIGHTNESS : TM_DEFAULT_BRIGHTNESS);
            prev_darktime = darktime;
        }
        
        counter = 0;
    }
    tm1637_display_dec(current_time, dots = !dots);
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