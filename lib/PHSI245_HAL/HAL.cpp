#include "HAL.h"
#include <Arduino.h>
#include <ch32x035.h>

uint16_t TouchState (0);

// Per-channel calibrated thresholds (loaded from EEPROM on v2)
static uint16_t touchThreshold[TOUCH_MAX_CHANNELS];
static uint16_t touchDebounce[TOUCH_MAX_CHANNELS];
static bool     touchCalibrated = false;

// Map pin name to ADC touch-key channel (TK0-TK9)
uint8_t pin_to_touch_adc(uint8_t pin)
{
    if (pin <= PA7) return pin; // PA0-PA7 -> ADC channels 0-7
#if HW_VERSION == 2
    if (pin == PB0) return 8;
    if (pin == PB1) return 9;
#endif
    return 0;
}

void initTouchButtons()
{
    // Set defaults for all channels
    for (uint8_t ch = 0; ch < TOUCH_MAX_CHANNELS; ch++) {
        touchThreshold[ch] = TOUCH_THRESHOLD_DEFAULT;
        touchDebounce[ch]  = TOUCH_DEBOUNCE_DEFAULT;
    }
    touchCalibrated = false;

    GPIO_InitTypeDef GPIO_InitStructure={0};
	ADC_InitTypeDef ADC_InitStructure={0};
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );

    // GPIOA pins: UP, LEFT, DOWN, RIGHT (PA0-PA3), plus A/B on v1 (PA5/PA4)
#if HW_VERSION == 1
    const unsigned char pa_pins[6] = {PIN_BTN_UP, PIN_BTN_LEFT, PIN_BTN_DOWN, PIN_BTN_RIGHT, PIN_BTN_A, PIN_BTN_B};
    const uint8_t pa_count = 6;
#else
    const unsigned char pa_pins[4] = {PIN_BTN_UP, PIN_BTN_LEFT, PIN_BTN_DOWN, PIN_BTN_RIGHT};
    const uint8_t pa_count = 4;
#endif
	for (uint8_t i = 0; i < pa_count; ++i)
	{
		GPIO_InitStructure.GPIO_Pin = (1 << pa_pins[i]);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}

#if HW_VERSION == 2
    // GPIOB pins: A, B (PB0-PB1, ADC channels 8-9)
    const uint16_t pb_pins[2] = {GPIO_Pin_0, GPIO_Pin_1};
    for (int i = 0; i < 2; ++ i)
    {
        GPIO_InitStructure.GPIO_Pin = pb_pins[i];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
#endif

    ADC_CLKConfig(ADC1, ADC_CLK_Div6);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Cmd(ADC1, ENABLE);
	TKey1->CTLR1 |= (1<<24);     // Enable TouchKey

}

// TODO: Add some hysteresis for debouncing.

uint16_t Touch_Key_Adc(uint8_t ch)
{
  ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_11Cycles );
  TKey1->IDATAR1 =0x80;  //Charging Time
  TKey1->RDATAR =0x8;   //Discharging Time
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));
  return (uint16_t) TKey1->RDATAR;
}

int IsTouched(unsigned long int channel)
{
    uint8_t adc_ch = pin_to_touch_adc(channel);
    uint16_t val = Touch_Key_Adc(adc_ch);
    uint16_t thresh = touchThreshold[adc_ch];
    uint16_t deb    = touchDebounce[adc_ch];

    if ( !(TouchState & (1 << adc_ch)) && val < thresh )
        {
            TouchState |= (1 << adc_ch);
            return 1;
        }
    if ( (TouchState & (1 << adc_ch)) && val > (thresh + deb) )
        {
            TouchState &= ~(1 << adc_ch);
            return 0;
        }
    return (TouchState & (1 << adc_ch));
}

#if HW_VERSION == 2
#include "storage.h"
#endif

// Packed calibration: 8-bit values (threshold/16, debounce/16) per channel.
// 6 channels used (PA0-PA3, PB0-PB1) = 12 bytes, fits in EEPROM.
struct CalibData {
    uint8_t threshold[6];
    uint8_t debounce[6];
};

// Map logical channels (0-5) to physical ADC channels
static uint8_t calibChannelMap[6];

void touchCalibrate()
{
    // Build channel map: PA0-PA3 (adc 0-3), PB0-PB1 (adc 8-9)
    calibChannelMap[0] = pin_to_touch_adc(PIN_BTN_UP);
    calibChannelMap[1] = pin_to_touch_adc(PIN_BTN_DOWN);
    calibChannelMap[2] = pin_to_touch_adc(PIN_BTN_LEFT);
    calibChannelMap[3] = pin_to_touch_adc(PIN_BTN_RIGHT);
    calibChannelMap[4] = pin_to_touch_adc(PIN_BTN_A);
    calibChannelMap[5] = pin_to_touch_adc(PIN_BTN_B);

#if HW_VERSION == 2
    // v2: try to load calibration from EEPROM first
    CalibData calib;
    if (storage::loadGame("TCH#", &calib, sizeof(calib)) == sizeof(calib)) {
        for (uint8_t i = 0; i < 6; i++) {
            uint8_t ch = calibChannelMap[i];
            touchThreshold[ch] = ((uint16_t)calib.threshold[i]) << 4;
            touchDebounce[ch]  = ((uint16_t)calib.debounce[i])  << 4;
        }
        touchCalibrated = true;
        return;
    }
#endif

    // Run calibration: measure baseline for each channel
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t ch = calibChannelMap[i];
        uint32_t sum = 0;
        for (uint8_t s = 0; s < 16; s++) {
            sum += Touch_Key_Adc(ch);
            delay(2);
        }
        uint16_t baseline = (uint16_t)(sum / 16);
        uint16_t thresh = (uint16_t)((uint32_t)baseline * 60 / 100);
        if (thresh < 0x200) thresh = 0x200;

        touchThreshold[ch] = thresh;
        touchDebounce[ch]  = thresh / 10;

#if HW_VERSION == 2
        calib.threshold[i] = (uint8_t)(thresh >> 4);
        calib.debounce[i]  = (uint8_t)(touchDebounce[ch] >> 4);
#endif
    }

#if HW_VERSION == 2
    storage::saveGame("TCH#", &calib, sizeof(calib));
#endif
    touchCalibrated = true;
}

bool touchIsCalibrated()
{
    return touchCalibrated;
}
