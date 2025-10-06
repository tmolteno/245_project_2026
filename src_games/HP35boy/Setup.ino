#include "main.h"

void setup()
{
  arduboy.boot();
  arduboy.setFrameRate(15);
  Serial.begin(9600);


  digitalWrite(PIN_LED_1, LOW);

  // Fake power on
  currentKey = keyboardSelectByCode(62);
  lcd_init(false);
  delay(500);

  if (arduboy.pressed(B_BUTTON))
  {
    enable_bug = true;
    digitalWrite(PIN_LED_1, HIGH);
    while (arduboy.pressed(B_BUTTON));
    digitalWrite(PIN_LED_1, LOW);
  }
}

boolean toggle;
void FlashLed()
{
  if (toggle)
    digitalWrite(PIN_LED_1, HIGH);
  if (!toggle)
    digitalWrite(PIN_LED_1, LOW);
  toggle = !toggle;
}
