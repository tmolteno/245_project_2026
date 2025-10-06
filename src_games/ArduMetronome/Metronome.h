#pragma once

#include <Arduboy2.h>
#include <stdint.h>

class Metronome
{

private:
  Arduboy2 arduboy;

public:
  void setup()
  {
    this->arduboy.begin();
    
    this->prepare();
  }

  void loop()
  {
    if(!this->arduboy.nextFrame())
      return;

    this->arduboy.pollButtons();

    this->arduboy.clear();
    
    this->update();
    
    this->render();

    this->arduboy.display();
  }
  
public:
  void update();
  void render();
  void prepare();

};
