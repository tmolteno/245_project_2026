#pragma once

#include <stdint.h>

namespace pong {

// Call once to initialize.
void init();

// Returns true if the game is currently playing (not on start screen).
bool isPlaying();

// Process one frame of input + physics.
// Call once per frame from the outside loop.
void update();

// Draw the current frame to the display buffer.
// Call after update(), before display().
void draw();

} // namespace pong
