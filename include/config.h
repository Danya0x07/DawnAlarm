#ifndef _CONFIG_H
#define _CONFIG_H

#ifndef DAWNALARM_MK
#   error Please specify the device version, DAWNALARM_MK=[1 - 2].
#endif

// UART, который, возможно, когда-нибудь будет использоваться.
#define SERIAL_GPORT    GPIOD
#define SERIAL_TX_GPIN  GPIO_PIN_5
#define SERIAL_RX_GPIN  GPIO_PIN_6

// Единственная кнопка, а больше и не надо.
#define BUTTON_GPORT    GPIOA
#define BUTTON_GPIN     GPIO_PIN_1

// Выводы 4х-разрядного 7-сегментного дисплея
#define TM1637_GPORT        GPIOA
#define TM1637_DIN_GPIN     GPIO_PIN_2
#define TM1637_CLK_GPIN     GPIO_PIN_3

// Вывод меандра с часов реального времени.
#define RTC_SQW_OUT_GPORT   GPIOC
#define RTC_SQW_OUT_GPIN    GPIO_PIN_3

#if (DAWNALARM_MK == 1)
    #define UNUSED_PINS_OF_PORTC   (GPIO_PIN_5)
    #ifdef DEBUG
    #   define UNUSED_PINS_OF_PORTD   (GPIO_PIN_2 | GPIO_PIN_4)
    #else
    #   define UNUSED_PINS_OF_PORTD   (GPIO_PIN_2 | GPIO_PIN_4 | SERIAL_TX_GPIN | SERIAL_RX_GPIN)
    #endif  //DEBUG

    #define POTENTIOMETER_ADC_CH    ADC1_CHANNEL_4
#endif  // DAWNALARM_MK == 1

#if (DAWNALARM_MK == 2)
    #ifndef DEBUG
    #   define UNUSED_PINS_OF_PORTD   (SERIAL_TX_GPIN | SERIAL_RX_GPIN)
    #endif  // DEBUG

    #define ENCODER_GPORT    GPIOD
    #define ENCODER_CHA_GPIN GPIO_PIN_4
    #define ENCODER_CHB_GPIN GPIO_PIN_3

    #define BUZZER_GPORT GPIOC
    #define BUZZER_GPIN  GPIO_PIN_5

    #define BATTERY_ADC_CH   ADC1_CHANNEL_3
#endif  // DAWNALARM_MK == 2

#endif  /* _CONFIG_H */