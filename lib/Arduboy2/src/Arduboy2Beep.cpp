/**
 * @file Arduboy2Beep.cpp
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#include <Arduino.h>
#include "Arduboy2Beep.h"
#include "HAL.h"

// BeepPin2 is a dummy, while BeepPin1 should work.

HardwareTimer tim3 (TIM3);

void timer_tick(void){
  digitalToggle(PIN_BEEP);
}

uint8_t BeepPin1::duration = 0;

void BeepPin1::begin()
{
  tim3.setPrescaleFactor(8);
  tim3.attachInterrupt(timer_tick);
  tim3.pause();
}

void BeepPin1::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin1::tone(uint16_t count, uint8_t dur)
{
  duration = dur;
  tim3.setOverflow(count, TICK_FORMAT);
  tim3.resume();
}

void BeepPin1::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepPin1::noTone()
{
  tim3.pause();
  duration = 0;
  digitalWrite(PIN_BEEP, PIN_BEEP_OFF);
}


uint8_t BeepPin2::duration = 0;

void BeepPin2::begin()
{
}

void BeepPin2::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin2::tone(uint16_t count, uint8_t dur)
{
  (void) count; // parameter not used

  duration = dur;
}

void BeepPin2::timer()
{
  if (duration) {
    --duration;
  }
}

void BeepPin2::noTone()
{
  duration = 0;
}
