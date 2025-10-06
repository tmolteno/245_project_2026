#include "HAL.h"

void DanCSound(uint8_t freq,uint8_t dur){
    if (freq==0) {delay(dur);return;}
    for (uint8_t t=0;t<dur;t++)
    {
        digitalToggle(PIN_BEEP);
        for (uint8_t t=0;t<(255-freq);t++)
            {_delay_us(1);}
        digitalToggle(PIN_BEEP);
        for (uint8_t t=0;t<(255-freq);t++)
            {_delay_us(1); }
    }
    digitalWrite(PIN_BEEP, PIN_BEEP_OFF);
}


#ifdef NO_GLOBAL_EEPROM

void EEPROMShim::update(int const idx, uint8_t const val)
{
    uint8_t storedData = read(idx);
    if (storedData == val)
            return;
    write(idx, val);
}

void EEPROMShim::begin(int a){
    EEPROMClass::begin();
}
void EEPROMShim::begin(){
    EEPROMClass::begin();
}

EEPROMShim EEPROM;
#endif
