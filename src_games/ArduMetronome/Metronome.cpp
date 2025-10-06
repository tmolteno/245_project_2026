// Some of this code is borrowed from victorman's Arduino Metronome: https://github.com/victorman/ArduinoMetronome

#include "Metronome.h"

#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <stdint.h>

#include "Images.h"
#include "Arrows.h"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

constexpr boolean ACCENT = true;
constexpr uint8_t halfScreenWidth = WIDTH / 2;
constexpr uint8_t halfScreenHeight = HEIGHT / 2;

// Beats Per Minute
uint8_t BPM = 60;

unsigned long last;
unsigned long tdelay;

// time signature, or number of beats per measure
// e.g. signature = 4 will beep once every four beats
uint8_t signature = 4;

int noteDuration;
int beat;

class Metronome;

void Metronome::prepare()
{
  
  beat = 0;
  //calculate seconds per beat
  tdelay = 60000/BPM;
  last = millis();
  
  // to calculate the note duration, take one second
  // divided by the note type.
  // this calculates how long the LED stays lit
  //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
  noteDuration = 1000 / 16;
}

void Metronome::update()
{
  int elapsed = millis() - last;
 
  // Adjust beats per minute by 1s
  if (arduboy.justPressed(UP_BUTTON) && BPM < 250)
  {
    ++BPM;
  }
  else if (arduboy.justPressed(DOWN_BUTTON) && BPM > 35)
  {
    --BPM;
  }
  
  if(BPM == 0) { 
    return;
  }

  // Adjust beats per minute by 10s
  if (arduboy.justPressed(B_BUTTON) && BPM < 250)
  {
    BPM += 10;
  }
  else if (arduboy.justPressed(A_BUTTON) && BPM > 40)
  {
    BPM -= 10;
  }
  
  if(BPM == 0) { 
    return;
  }
  
  // Adjust time signature by single digits
  if (arduboy.justPressed(RIGHT_BUTTON) && signature < 7)
  {
    ++signature;
  }
  else if (arduboy.justPressed(LEFT_BUTTON) && signature > 2)
  {
    --signature;
  }
  
  if(BPM == 0) { 
    return;
  }
  
  // Keep LED off every beat
  if (elapsed > noteDuration)
  {
    arduboy.setRGBled(0, 0, 0);
  }
  
  tdelay = 60000/BPM;
  
  if (elapsed < tdelay)
  { 
    return;
  }
  
  int play_note = NOTE_C4;
 
  // Figure out when to play higher pitch beep
  // at beginning of measure
  beat = beat % signature;
  
  if (ACCENT && beat == 0) 
  {
    play_note = NOTE_C6;
    arduboy.setRGBled(255, 0, 0);
  }

  sound.tone(play_note, 50);
  arduboy.setRGBled(GREEN_LED, 35);
  
  last = millis();
  beat++;
}

void Metronome::render()
{
  // Print credits
  arduboy.setCursor(halfScreenWidth - 16, 0);
  arduboy.print(F("ArduMetronome"));
  
  // Show beats per minute
  arduboy.setCursor(0, halfScreenHeight - 10);
  arduboy.setTextSize(3);
  arduboy.print(BPM);
  arduboy.setTextSize(1);
  arduboy.print(F("BPM"));
  
  // Show time signature (AKA beats)
  arduboy.setCursor(halfScreenWidth + 10, halfScreenHeight - 10);
  arduboy.setTextSize(3);
  arduboy.print(signature);
  arduboy.setTextSize(1);
  arduboy.print(F("BEATS"));
  
  for (uint8_t i = 0; i < 4; ++i)
  {
    arrows[i].frame = 0;
    
    if (arduboy.pressed(UP_BUTTON))
    {
      arrows[0].frame = 1;
    }
    
    if (arduboy.pressed(DOWN_BUTTON))
    {
      arrows[1].frame = 1;
    }
    
    if (arduboy.pressed(LEFT_BUTTON))
    {
      arrows[2].frame = 1;
    }
    
    if (arduboy.pressed(RIGHT_BUTTON))
    {
      arrows[3].frame = 1;
    }

    Sprites::drawPlusMask(arrows[i].x, arrows[i].y, arrows[i].image, arrows[i].frame);
  }
}
