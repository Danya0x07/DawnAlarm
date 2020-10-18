#include "halutils.h"

static ErrorStatus _I2C_CheckEvent(I2C_Event_TypeDef I2C_Event);

void delay_ms(uint16_t ms)
{
    while (ms--) {
        // задержка на 1ms
        tim4_set_counter(0);
        while (tim4_get_counter() < 125)
            ;
    }
}

void adc_start_conversion(void)
{
    ADC1->CR1 |= ADC1_CR1_ADON;
}

bool adc_conversion_complete(void)
{
    return !!(ADC1->CSR & ADC1_FLAG_EOC);
}

uint16_t adc_read_value(void)
{
    uint16_t temph = 0;
    uint8_t templ = 0;

    templ = ADC1->DRL;
    temph = ADC1->DRH;
    temph = temph << 8 | templ;

    return temph;
}

static void i2c_start(uint8_t addr, uint8_t direction)
{
    I2C->CR2 |= I2C_CR2_START;
    while (!_I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C->DR = addr & 0xFE | direction & 0x01;  // Отправляем адрес ведомого.
    if (direction == 0)
        while (!_I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    else
        while (!_I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
}

static void i2c_stop(void)
{
    I2C->CR2 |= I2C_CR2_STOP;
}

static void i2c_write(const uint8_t *data, uint8_t len)
{
    while (len--) {
        I2C->DR = *data++;
        while (!_I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
}

static void i2c_read(uint8_t *data, uint8_t len)
{
    // Включаем отправку автоподтверждений
    I2C->CR2 |= I2C_CR2_ACK;
    I2C->CR2 &= ~I2C_CR2_POS;
    for (; len; len--) {
        if (len == 1) {
            // Выключаем отправку подтверждений.
            I2C->CR2 &= ~I2C_CR2_ACK;
        }
        while (!_I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));
        *data++ = I2C->DR;
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

/* Скопировано без изменений (за искл. форматирования и названия) из STM8S_StdPeriph_Driver.
 * Поскольку компилятор SDCC пришивает библиотеки целиком, даже если
 * из неё используется всего несколько функций, вынос используемого
 * кода в отдельный файл позволит значительно сэкономить память МК.
 */

static void _I2C_AcknowledgeConfig(I2C_Ack_TypeDef Ack)
{
    /* Check function parameters */
    assert_param(IS_I2C_ACK_OK(Ack));

    if (Ack == I2C_ACK_NONE)
    {
        /* Disable the acknowledgement */
        I2C->CR2 &= (uint8_t)(~I2C_CR2_ACK);
    }
    else
    {
        /* Enable the acknowledgement */
        I2C->CR2 |= I2C_CR2_ACK;

        if (Ack == I2C_ACK_CURR)
        {
            /* Configure (N)ACK on current byte */
            I2C->CR2 &= (uint8_t)(~I2C_CR2_POS);
        }
        else
        {
            /* Configure (N)ACK on next byte */
            I2C->CR2 |= I2C_CR2_POS;
        }
    }
}

void _I2C_Init(uint32_t OutputClockFrequencyHz, uint16_t OwnAddress,
              I2C_DutyCycle_TypeDef I2C_DutyCycle, I2C_Ack_TypeDef Ack,
              I2C_AddMode_TypeDef AddMode, uint8_t InputClockFrequencyMHz)
{
    uint16_t result = 0x0004;
    uint16_t tmpval = 0;
    uint8_t tmpccrh = 0;

    /* Check the parameters */
    assert_param(IS_I2C_ACK_OK(Ack));
    assert_param(IS_I2C_ADDMODE_OK(AddMode));
    assert_param(IS_I2C_OWN_ADDRESS_OK(OwnAddress));
    assert_param(IS_I2C_DUTYCYCLE_OK(I2C_DutyCycle));
    assert_param(IS_I2C_INPUT_CLOCK_FREQ_OK(InputClockFrequencyMHz));
    assert_param(IS_I2C_OUTPUT_CLOCK_FREQ_OK(OutputClockFrequencyHz));

    /*------------------------- I2C FREQ Configuration ------------------------*/
    /* Clear frequency bits */
    I2C->FREQR &= (uint8_t)(~I2C_FREQR_FREQ);
    /* Write new value */
    I2C->FREQR |= InputClockFrequencyMHz;

    /*--------------------------- I2C CCR Configuration ------------------------*/
    /* Disable I2C to configure TRISER */
    I2C->CR1 &= (uint8_t)(~I2C_CR1_PE);

    /* Clear CCRH & CCRL */
    I2C->CCRH &= (uint8_t)(~(I2C_CCRH_FS | I2C_CCRH_DUTY | I2C_CCRH_CCR));
    I2C->CCRL &= (uint8_t)(~I2C_CCRL_CCR);

    /* Detect Fast or Standard mode depending on the Output clock frequency selected */
    if (OutputClockFrequencyHz > I2C_MAX_STANDARD_FREQ) /* FAST MODE */
    {
        /* Set F/S bit for fast mode */
        tmpccrh = I2C_CCRH_FS;

        if (I2C_DutyCycle == I2C_DUTYCYCLE_2)
        {
            /* Fast mode speed calculate: Tlow/Thigh = 2 */
            result = (uint16_t)((InputClockFrequencyMHz * 1000000) / (OutputClockFrequencyHz * 3));
        }
        else /* I2C_DUTYCYCLE_16_9 */
        {
            /* Fast mode speed calculate: Tlow/Thigh = 16/9 */
            result = (uint16_t)((InputClockFrequencyMHz * 1000000) / (OutputClockFrequencyHz * 25));
            /* Set DUTY bit */
            tmpccrh |= I2C_CCRH_DUTY;
        }

        /* Verify and correct CCR value if below minimum value */
        if (result < (uint16_t)0x01)
        {
            /* Set the minimum allowed value */
            result = (uint16_t)0x0001;
        }

        /* Set Maximum Rise Time: 300ns max in Fast Mode
    = [300ns/(1/InputClockFrequencyMHz.10e6)]+1
    = [(InputClockFrequencyMHz * 3)/10]+1 */
        tmpval = ((InputClockFrequencyMHz * 3) / 10) + 1;
        I2C->TRISER = (uint8_t)tmpval;
    }
    else /* STANDARD MODE */
    {

        /* Calculate standard mode speed */
        result = (uint16_t)((InputClockFrequencyMHz * 1000000) / (OutputClockFrequencyHz << (uint8_t)1));

        /* Verify and correct CCR value if below minimum value */
        if (result < (uint16_t)0x0004)
        {
            /* Set the minimum allowed value */
            result = (uint16_t)0x0004;
        }

        /* Set Maximum Rise Time: 1000ns max in Standard Mode
    = [1000ns/(1/InputClockFrequencyMHz.10e6)]+1
    = InputClockFrequencyMHz+1 */
        I2C->TRISER = (uint8_t)(InputClockFrequencyMHz + (uint8_t)1);
    }

    /* Write CCR with new calculated value */
    I2C->CCRL = (uint8_t)result;
    I2C->CCRH = (uint8_t)((uint8_t)((uint8_t)(result >> 8) & I2C_CCRH_CCR) | tmpccrh);

    /* Enable I2C */
    I2C->CR1 |= I2C_CR1_PE;

    /* Configure I2C acknowledgement */
    _I2C_AcknowledgeConfig(Ack);

    /*--------------------------- I2C OAR Configuration ------------------------*/
    I2C->OARL = (uint8_t)(OwnAddress);
    I2C->OARH = (uint8_t)((uint8_t)(AddMode | I2C_OARH_ADDCONF) |
                          (uint8_t)((OwnAddress & (uint16_t)0x0300) >> (uint8_t)7));
}

static ErrorStatus _I2C_CheckEvent(I2C_Event_TypeDef I2C_Event)
{
    __IO uint16_t lastevent = 0x00;
    uint8_t flag1 = 0x00;
    uint8_t flag2 = 0x00;
    ErrorStatus status = ERROR;

    /* Check the parameters */
    assert_param(IS_I2C_EVENT_OK(I2C_Event));

    if (I2C_Event == I2C_EVENT_SLAVE_ACK_FAILURE)
    {
        lastevent = I2C->SR2 & I2C_SR2_AF;
    }
    else
    {
        flag1 = I2C->SR1;
        flag2 = I2C->SR3;
        lastevent = ((uint16_t)((uint16_t)flag2 << (uint16_t)8) | (uint16_t)flag1);
    }
    /* Check whether the last event is equal to I2C_EVENT */
    if (((uint16_t)lastevent & (uint16_t)I2C_Event) == (uint16_t)I2C_Event)
    {
        /* SUCCESS: last event is equal to I2C_EVENT */
        status = SUCCESS;
    }
    else
    {
        /* ERROR: last event is different from I2C_EVENT */
        status = ERROR;
    }

    /* Return status */
    return status;
}

void _TIM1_TimeBaseInit(uint16_t TIM1_Prescaler,
                       TIM1_CounterMode_TypeDef TIM1_CounterMode,
                       uint16_t TIM1_Period,
                       uint8_t TIM1_RepetitionCounter)
{
    /* Check parameters */
    assert_param(IS_TIM1_COUNTER_MODE_OK(TIM1_CounterMode));

    /* Set the Autoreload value */
    TIM1->ARRH = (uint8_t)(TIM1_Period >> 8);
    TIM1->ARRL = (uint8_t)(TIM1_Period);

    /* Set the Prescaler value */
    TIM1->PSCRH = (uint8_t)(TIM1_Prescaler >> 8);
    TIM1->PSCRL = (uint8_t)(TIM1_Prescaler);

    /* Select the Counter Mode */
    TIM1->CR1 = (uint8_t)((uint8_t)(TIM1->CR1 & (uint8_t)(~(TIM1_CR1_CMS | TIM1_CR1_DIR))) | (uint8_t)(TIM1_CounterMode));

    /* Set the Repetition Counter value */
    TIM1->RCR = TIM1_RepetitionCounter;
}

void _TIM1_OC1Init(TIM1_OCMode_TypeDef TIM1_OCMode,
                  TIM1_OutputState_TypeDef TIM1_OutputState,
                  TIM1_OutputNState_TypeDef TIM1_OutputNState,
                  uint16_t TIM1_Pulse,
                  TIM1_OCPolarity_TypeDef TIM1_OCPolarity,
                  TIM1_OCNPolarity_TypeDef TIM1_OCNPolarity,
                  TIM1_OCIdleState_TypeDef TIM1_OCIdleState,
                  TIM1_OCNIdleState_TypeDef TIM1_OCNIdleState)
{
    /* Check the parameters */
    assert_param(IS_TIM1_OC_MODE_OK(TIM1_OCMode));
    assert_param(IS_TIM1_OUTPUT_STATE_OK(TIM1_OutputState));
    assert_param(IS_TIM1_OUTPUTN_STATE_OK(TIM1_OutputNState));
    assert_param(IS_TIM1_OC_POLARITY_OK(TIM1_OCPolarity));
    assert_param(IS_TIM1_OCN_POLARITY_OK(TIM1_OCNPolarity));
    assert_param(IS_TIM1_OCIDLE_STATE_OK(TIM1_OCIdleState));
    assert_param(IS_TIM1_OCNIDLE_STATE_OK(TIM1_OCNIdleState));

    /* Disable the Channel 1: Reset the CCE Bit, Set the Output State , 
  the Output N State, the Output Polarity & the Output N Polarity*/
    TIM1->CCER1 &= (uint8_t)(~(TIM1_CCER1_CC1E | TIM1_CCER1_CC1NE | TIM1_CCER1_CC1P | TIM1_CCER1_CC1NP));
    /* Set the Output State & Set the Output N State & Set the Output Polarity &
  Set the Output N Polarity */
    TIM1->CCER1 |= (uint8_t)((uint8_t)((uint8_t)(TIM1_OutputState & TIM1_CCER1_CC1E) | (uint8_t)(TIM1_OutputNState & TIM1_CCER1_CC1NE)) | (uint8_t)((uint8_t)(TIM1_OCPolarity & TIM1_CCER1_CC1P) | (uint8_t)(TIM1_OCNPolarity & TIM1_CCER1_CC1NP)));

    /* Reset the Output Compare Bits & Set the Output Compare Mode */
    TIM1->CCMR1 = (uint8_t)((uint8_t)(TIM1->CCMR1 & (uint8_t)(~TIM1_CCMR_OCM)) |
                            (uint8_t)TIM1_OCMode);

    /* Reset the Output Idle state & the Output N Idle state bits */
    TIM1->OISR &= (uint8_t)(~(TIM1_OISR_OIS1 | TIM1_OISR_OIS1N));
    /* Set the Output Idle state & the Output N Idle state configuration */
    TIM1->OISR |= (uint8_t)((uint8_t)(TIM1_OCIdleState & TIM1_OISR_OIS1) |
                            (uint8_t)(TIM1_OCNIdleState & TIM1_OISR_OIS1N));

    /* Set the Pulse value */
    TIM1->CCR1H = (uint8_t)(TIM1_Pulse >> 8);
    TIM1->CCR1L = (uint8_t)(TIM1_Pulse);
}

void _TIM1_OC2Init(TIM1_OCMode_TypeDef TIM1_OCMode,
                  TIM1_OutputState_TypeDef TIM1_OutputState,
                  TIM1_OutputNState_TypeDef TIM1_OutputNState,
                  uint16_t TIM1_Pulse,
                  TIM1_OCPolarity_TypeDef TIM1_OCPolarity,
                  TIM1_OCNPolarity_TypeDef TIM1_OCNPolarity,
                  TIM1_OCIdleState_TypeDef TIM1_OCIdleState,
                  TIM1_OCNIdleState_TypeDef TIM1_OCNIdleState)
{
    /* Check the parameters */
    assert_param(IS_TIM1_OC_MODE_OK(TIM1_OCMode));
    assert_param(IS_TIM1_OUTPUT_STATE_OK(TIM1_OutputState));
    assert_param(IS_TIM1_OUTPUTN_STATE_OK(TIM1_OutputNState));
    assert_param(IS_TIM1_OC_POLARITY_OK(TIM1_OCPolarity));
    assert_param(IS_TIM1_OCN_POLARITY_OK(TIM1_OCNPolarity));
    assert_param(IS_TIM1_OCIDLE_STATE_OK(TIM1_OCIdleState));
    assert_param(IS_TIM1_OCNIDLE_STATE_OK(TIM1_OCNIdleState));

    /* Disable the Channel 1: Reset the CCE Bit, Set the Output State , 
  the Output N State, the Output Polarity & the Output N Polarity*/
    TIM1->CCER1 &= (uint8_t)(~(TIM1_CCER1_CC2E | TIM1_CCER1_CC2NE |
                               TIM1_CCER1_CC2P | TIM1_CCER1_CC2NP));

    /* Set the Output State & Set the Output N State & Set the Output Polarity &
  Set the Output N Polarity */
    TIM1->CCER1 |= (uint8_t)((uint8_t)((uint8_t)(TIM1_OutputState & TIM1_CCER1_CC2E) |
                                       (uint8_t)(TIM1_OutputNState & TIM1_CCER1_CC2NE)) |
                             (uint8_t)((uint8_t)(TIM1_OCPolarity & TIM1_CCER1_CC2P) |
                                       (uint8_t)(TIM1_OCNPolarity & TIM1_CCER1_CC2NP)));

    /* Reset the Output Compare Bits & Set the Output Compare Mode */
    TIM1->CCMR2 = (uint8_t)((uint8_t)(TIM1->CCMR2 & (uint8_t)(~TIM1_CCMR_OCM)) |
                            (uint8_t)TIM1_OCMode);

    /* Reset the Output Idle state & the Output N Idle state bits */
    TIM1->OISR &= (uint8_t)(~(TIM1_OISR_OIS2 | TIM1_OISR_OIS2N));
    /* Set the Output Idle state & the Output N Idle state configuration */
    TIM1->OISR |= (uint8_t)((uint8_t)(TIM1_OISR_OIS2 & TIM1_OCIdleState) |
                            (uint8_t)(TIM1_OISR_OIS2N & TIM1_OCNIdleState));

    /* Set the Pulse value */
    TIM1->CCR2H = (uint8_t)(TIM1_Pulse >> 8);
    TIM1->CCR2L = (uint8_t)(TIM1_Pulse);
}

void _TIM1_OC4Init(TIM1_OCMode_TypeDef TIM1_OCMode,
                  TIM1_OutputState_TypeDef TIM1_OutputState,
                  uint16_t TIM1_Pulse,
                  TIM1_OCPolarity_TypeDef TIM1_OCPolarity,
                  TIM1_OCIdleState_TypeDef TIM1_OCIdleState)
{
    /* Check the parameters */
    assert_param(IS_TIM1_OC_MODE_OK(TIM1_OCMode));
    assert_param(IS_TIM1_OUTPUT_STATE_OK(TIM1_OutputState));
    assert_param(IS_TIM1_OC_POLARITY_OK(TIM1_OCPolarity));
    assert_param(IS_TIM1_OCIDLE_STATE_OK(TIM1_OCIdleState));

    /* Disable the Channel 4: Reset the CCE Bit */
    TIM1->CCER2 &= (uint8_t)(~(TIM1_CCER2_CC4E | TIM1_CCER2_CC4P));
    /* Set the Output State  &  the Output Polarity */
    TIM1->CCER2 |= (uint8_t)((uint8_t)(TIM1_OutputState & TIM1_CCER2_CC4E) |
                             (uint8_t)(TIM1_OCPolarity & TIM1_CCER2_CC4P));

    /* Reset the Output Compare Bit  and Set the Output Compare Mode */
    TIM1->CCMR4 = (uint8_t)((uint8_t)(TIM1->CCMR4 & (uint8_t)(~TIM1_CCMR_OCM)) |
                            TIM1_OCMode);

    /* Set the Output Idle state */
    if (TIM1_OCIdleState != TIM1_OCIDLESTATE_RESET)
    {
        TIM1->OISR |= (uint8_t)(~TIM1_CCER2_CC4P);
    }
    else
    {
        TIM1->OISR &= (uint8_t)(~TIM1_OISR_OIS4);
    }

    /* Set the Pulse value */
    TIM1->CCR4H = (uint8_t)(TIM1_Pulse >> 8);
    TIM1->CCR4L = (uint8_t)(TIM1_Pulse);
}

void _TIM1_SetCompare1(uint16_t Compare1)
{
    /* Set the Capture Compare1 Register value */
    TIM1->CCR1H = (uint8_t)(Compare1 >> 8);
    TIM1->CCR1L = (uint8_t)(Compare1);
}

void _TIM1_SetCompare2(uint16_t Compare2)
{
    /* Set the Capture Compare2 Register value */
    TIM1->CCR2H = (uint8_t)(Compare2 >> 8);
    TIM1->CCR2L = (uint8_t)(Compare2);
}

void _TIM1_SetCompare4(uint16_t Compare4)
{
    /* Set the Capture Compare4 Register value */
    TIM1->CCR4H = (uint8_t)(Compare4 >> 8);
    TIM1->CCR4L = (uint8_t)(Compare4);
}

uint16_t _TIM1_GetCapture1(void)
{
    /* Get the Capture 1 Register value */

    uint16_t tmpccr1 = 0;
    uint8_t tmpccr1l = 0, tmpccr1h = 0;

    tmpccr1h = TIM1->CCR1H;
    tmpccr1l = TIM1->CCR1L;

    tmpccr1 = (uint16_t)(tmpccr1l);
    tmpccr1 |= (uint16_t)((uint16_t)tmpccr1h << 8);
    /* Get the Capture 1 Register value */
    return (uint16_t)tmpccr1;
}

uint16_t _TIM1_GetCapture2(void)
{
    /* Get the Capture 2 Register value */

    uint16_t tmpccr2 = 0;
    uint8_t tmpccr2l = 0, tmpccr2h = 0;

    tmpccr2h = TIM1->CCR2H;
    tmpccr2l = TIM1->CCR2L;

    tmpccr2 = (uint16_t)(tmpccr2l);
    tmpccr2 |= (uint16_t)((uint16_t)tmpccr2h << 8);
    /* Get the Capture 2 Register value */
    return (uint16_t)tmpccr2;
}

uint16_t _TIM1_GetCapture4(void)
{
    /* Get the Capture 4 Register value */
    uint16_t tmpccr4 = 0;
    uint8_t tmpccr4l = 0, tmpccr4h = 0;

    tmpccr4h = TIM1->CCR4H;
    tmpccr4l = TIM1->CCR4L;

    tmpccr4 = (uint16_t)(tmpccr4l);
    tmpccr4 |= (uint16_t)((uint16_t)tmpccr4h << 8);
    /* Get the Capture 4 Register value */
    return (uint16_t)tmpccr4;
}
