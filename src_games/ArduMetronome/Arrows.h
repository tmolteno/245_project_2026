#pragma once

struct Arrow
{
  uint8_t x;
  uint8_t y;
  const uint8_t *image;
  uint8_t frame;
};

Arrow arrows[4] = {
  { 16, 0, upArrow_plus_mask, 0 },
  { 16, 48, downArrow_plus_mask, 0 },
  { 80, 48, leftArrow_plus_mask, 0 },
  { 100, 48, rightArrow_plus_mask, 0 },
};
