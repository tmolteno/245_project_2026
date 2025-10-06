/*
 * Tiny 1010 - a 1010 block puzzle game for the Arduboy.
 *
 * Copyright (c) 2016, Mike Meyer (mwm@mired.org)
 *
 * Modifications to lower the LED brightness
 * Copyright (c) 2016, Scott Allen
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * The 1010 block puzzle game involes placings shapes composed of square blocks
 * on a 10 x 10 game board so that they don't overlap with previously dropped
 * shapes. When a row or column is filled, the blocks on it are cleared and points
 * are scored. Bonus points for clearing multiple rows or columns with a single
 * point.
 *
 * Shapes are presented in groups of three. After all three have been placed,
 * another set of three is presented. If it isn't possible to place any existing
 * block, the game ends.
 */

#include <EEPROM.h>
#include <Arduboy2.h>

Arduboy2 arduboy;

/* Data structures */

// LED brightness
// off = 0. on value can be 1 for dimmest to 255 for brightest
const byte RGB_LED_OFF = 0;
const byte RED_LED_ON = 5;
const byte GREEN_LED_ON = 5;
const byte BLUE_LED_ON = 20;

// The  board
const byte BOARD_SIZE = 10;	// a board_size by board_size game
const byte BLOCK_SIZE = 6;	// Each tile is tile_size pixels on a side.
const byte BOARD_X = 62;	// Left edge of board
const byte BOARD_Y = 2;		// Top edge of the board.
const byte EMPTY = 0;		// Name of the empty tile.
const byte CLEAR = 1;		// WHITE tile for clearing.
const byte REMOVED = 255;	// A tile about to be emptied.

// Tile bitmaps. All 7x7, a 1-pixel border around a 5x5 color.
const byte TILE_SIZE = 7;			// Size of a color tile to draw.
const byte colors[][TILE_SIZE] PROGMEM = {
     {0x7F, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7F},// EMPTY must be in position 0.
     {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F},// 3x3 square, CLEAR in position 1.
     {0x7F, 0x41, 0x5D, 0x5D, 0x5D, 0x41, 0x7F},// 2x2 square
     {0x7F, 0x41, 0x41, 0x49, 0x41, 0x41, 0x7F},// 1x1 square
     {0x7F, 0x7F, 0x7F, 0x41, 0x7F, 0x7F, 0x7F},// vertical 5 long
     {0x7F, 0x41, 0x7F, 0x7F, 0x7F, 0x41, 0x7F},// vertical 4 long
     {0x7F, 0x41, 0x7F, 0x41, 0x7F, 0x41, 0x7F},// vertical 3 long
     {0x7F, 0x41, 0x41, 0x7F, 0x41, 0x41, 0x7F},// vertical 2 long
     {0x7F, 0x77, 0x77, 0x77, 0x77, 0x77, 0x7F},// horizontal 4 long
     {0x7F, 0x5D, 0x5D, 0x5D, 0x5D, 0x5D, 0x7F},// horizontal 4 long
     {0x7F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7F},// horizontal 3 long
     {0x7F, 0x49, 0x49, 0x49, 0x49, 0x49, 0x7F},// horizontal 2 long
     {0x7F, 0x7F, 0x63, 0x6B, 0x63, 0x7F, 0x7F},// 3x3 corners
     {0x7F, 0x41, 0x5D, 0x55, 0x5D, 0x41, 0x7F},// 2x2 corners
     {0x7F, 0x55, 0x6B, 0x55, 0x6B, 0x55, 0x7F},// L shapes
};


// The shapes
const byte ss = 3;	// 1x1 square

const byte threes[][3] = {
     { 6,  6,  6},	// vertical 3 in a row
     { 7,  7,  0},	// vertical 2 in a row
     {10, 10, 10},	// horizontal 3 in a row
     {11, 11, 0},	// horizontal 2 in a row
};

const byte fours[][4] = {
     {5, 5, 5, 5},	// vertical 4 in a row
     {9, 9, 9, 9},	// horizontal 4 in a row
     {2, 2, 2, 2},	// 2x2 square
     {0, 13, 13, 13},	// 4 2x2 corners
     {13, 0, 13, 13},
     {13, 13, 0, 13},
     {13, 13, 13, 0},
};

const byte sixes[][6] = {
     {14, 14, 14, 14,  0,  0},	// 8 L shapes is to many!
     {14, 14, 14,  0,  0, 14},
     {14, 0, 0,  14,  14, 14},
     { 0, 0, 14, 14,  14, 14},
     {14, 14,  0, 14,  0, 14},
     {14, 14, 14,  0, 14,  0},
     {14,  0, 14,  0, 14, 14},
     { 0, 14, 0,  14, 14, 14},
     { 4,  4,  4,  4,  4,  0},	// vertical 5 in a row
     { 8,  8,  8,  8,  8,  0},	// horizontal 5 in a row 
};
     

const byte nines[][9] = {
     { 1,  1,  1,  1,  1,  1,  1,  1,  1},	// 3x3 square
     {12, 12, 12, 12,  0,  0, 12,  0,  0},	// 4 3x3 corners
     {12, 12, 12,  0,  0, 12,  0,  0, 12},
     { 0,  0, 12,  0,  0, 12, 12, 12, 12},
     {12,  0,  0, 12,  0,  0, 12, 12, 12},
};

typedef struct {
     const byte width, height;	// Rectangle it fits in
     const byte * const tiles; 	// width x height shape of colors.
} shape;

const shape shapes[] = {
     {3, 3, nines[0]},	// 3x3 square
     {2, 2, fours[2]},  // 2x2 square
     {1, 1, &ss},	// 1x1 square
     {1, 5, sixes[8]},	// Vertical lines 5 through 2
     {1, 4, fours[0]},
     {1, 3, threes[0]},
     {1, 2, threes[1]},
     {5, 1, sixes[9]},	// Horizontal lines 5 through 2
     {4, 1, fours[1]},
     {3, 1, threes[2]},
     {2, 1, threes[3]},
     {3, 3, nines[1]},	// large 3x3 corners
     {3, 3, nines[2]},
     {3, 3, nines[3]},
     {3, 3, nines[4]},
     {3, 2, sixes[0]},	// L shaped corners. Use 4 of 8.
     {3, 2, sixes[1]},
//     {3, 2, sixes[2]},
//     {3, 2, sixes[3]},
//     {2, 3, sixes[4]},
//     {2, 3, sixes[5]},
     {2, 3, sixes[6]},
     {2, 3, sixes[7]},
     {2, 2, fours[3]},	// small 2x2 corners
     {2, 2, fours[4]},
     {2, 2, fours[5]},
     {2, 2, fours[6]},
};


/* Help screens - 8 lines of 20 characters on each screen */
char help_buffer[161];	// buffer for it.
const char nav_help[] PROGMEM =
     "  Help Navigation\n\nPage back and forth\nwith \33 and \32 as\n"
     "indicated in the\nupper corners. Exit\nwith \30. Press \32 now.";
const char play_help[] PROGMEM =
     "      Game play\n\nSelect shapes to\nplace on empty\nsquares on the 10x10\n"
     "game board. Filling a\nrow or column clears\nit.";
const char score_help[] PROGMEM =
     "       Scoring\n\nPlacing a shape\nscores one point for\neach block in the\n"
     "shape, and ten points\nfor each line\ncleared.";
const char key_help[] PROGMEM =
     "  Buttons: B pauses!\n"
     " Selecting\336  Moving\n"
     "\30\31 changes\336 \33\31\30\32\n"
     "shape     \336 moves\n"
     "\32 selects \336 \33 at left\n"
     "to corner \336 deselects\n"
     "A selects \336 A places\n"
     "& autofits\336 shape";
const char about[] PROGMEM = 
     "        About\n\n1010 c Mike Meyer\nmwm@mired.org\n\nhttps://goo.gl/9EeMnN";

const char *const help[] PROGMEM = {
     nav_help, play_help, score_help, key_help, about
};


// The game
typedef enum {INIT, MENU, DONE, SELECTING, MOVING, PAGING} game_state;
const byte WAIT_LEN = 3;

struct game_data {
     unsigned long high, score;
     byte delay;
     int8_t waiting[WAIT_LEN];	// Shapes ready to be played.
     byte x, y;	// And it's location on the board.
     byte board[BOARD_SIZE][BOARD_SIZE];
} game;

// Ephemeral values.
game_state state;		// Current game state
byte selected;	        	// substate indicator

const char *prompt;		// Menu prompt
#define PAUSED		"  Paused"
#define GAMEOVER	"Game over!"

// The menu
enum {HELP, RESUME, NEW, KEYS, MENU_LAST};

/* Code to play the game. */
// Returns true if the shape is over clear tiles.
boolean
shape_clear(const shape *const s, const byte x, const byte y) {
     if (x + s->width > BOARD_SIZE || y + s->height > BOARD_SIZE)
          return 0;
     for (byte j = 0; j < s->height; j += 1)
          for (byte i = 0; i < s->width; i += 1)
               if (s->tiles[i + j * s->width] != EMPTY
               && game.board[x + i][y + j] != EMPTY)
                    return 0;
     return 1;
}

boolean
find_clear(const shape *const s) {
     for (game.x = 0; game.x <= BOARD_SIZE - s->width; game.x += 1)
          for (game.y = 0; game.y <= BOARD_SIZE - s->height; game.y += 1)
               if (shape_clear(s, game.x, game.y))
                    return 1;
     return 0;
}


// Place a shape on the board
void
new_shapes(void) {
     for (byte i = 0; i < WAIT_LEN; i += 1) {
          game.waiting[i] = random(sizeof(shapes) / sizeof(shape));
          for (byte j = 0; j < i; j += 1)
               if (game.waiting[i] == game.waiting[j]) {
                    i -= 1;
                    break;
               }
     }
     selected = 0;
}

void
select_next(void) {
     for (byte x = 0; x < WAIT_LEN; x += 1)
          if (game.waiting[x] >= 0) {
               selected = x;
               return;
          }
     new_shapes();
}

void
make_move(void)  {
     const shape *const s = &shapes[game.waiting[selected]];
     byte x;

     for (byte y = 0; y < s->height; y += 1)
          for (byte x = 0; x < s->width; x += 1)
               if (s->tiles[x + y * s->width] != EMPTY) {
                    game.score += 1;
                    game.board[game.x + x][game.y + y] 
                         = s->tiles[x + y * s->width];
               }
     for (byte y = 0; y < BOARD_SIZE; y += 1) {
          for (x = 0; x < BOARD_SIZE; x += 1)
               if (game.board[x][y] == EMPTY)
                    break;
          if (x >= BOARD_SIZE) {
               game.score += 10;
               for (byte x = 0; x < BOARD_SIZE; x += 1)
                    game.board[x][y] = REMOVED;
          }
          for (x = 0; x < BOARD_SIZE; x += 1)
               if (game.board[y][x] == EMPTY)
                    break;
          if (x >= BOARD_SIZE) {
               game.score += 10;
               for (byte x = 0; x < BOARD_SIZE; x += 1)
                    game.board[y][x] = REMOVED;
          }
     }
     if (game.score > game.high)
          game.high = game.score;
     for (byte y = 0; y < BOARD_SIZE; y += 1)
          for (byte x = 0; x < BOARD_SIZE; x += 1)
               if (game.board[x][y] == REMOVED)
                    game.board[x][y] = EMPTY;

     game.waiting[selected] = -1;
     select_next();

     for (byte w = 0; w < WAIT_LEN; w += 1)
          if (game.waiting[w] >= 0 && find_clear(&shapes[game.waiting[w]])) {
               state = SELECTING;
               return;
          }
     state = DONE;
     selected = NEW;
     prompt = GAMEOVER;
}


void
reset_game(void) {
     game.delay = 100;
     game.score = 0;
     for (byte i = 0; i < BOARD_SIZE; i += 1)
          for (byte j = 0; j < BOARD_SIZE; j += 1)
               game.board[i][j] = EMPTY;

     new_shapes();
     state = SELECTING;
}

void
do_menu(void) {
     switch (selected) {
     case NEW:
          reset_game();
          // Fall through to
     case RESUME:
          state = SELECTING;
          select_next();
          break;
     case HELP:
          state = PAGING;
          selected = HELP;
          break;
     case KEYS:
          state = PAGING;
          selected = KEYS;
          break;
     }
}

/* Code to draw things on the display */
// Coloring in things that aren't on the board.
void
color_tile(const byte color, const byte x, const byte y) {
     arduboy.drawBitmap(x, y, colors[color % (sizeof(colors) / TILE_SIZE)],
                        TILE_SIZE, TILE_SIZE);
}

void
color_shape(const shape *const s, const byte x, const byte y) {
     for (byte j = 0; j < s->height; j += 1)
          for (byte i = 0; i < s->width; i += 1)
               if (s->tiles[i + j * s->width] != EMPTY) {
                    arduboy.drawBitmap(x + i * BLOCK_SIZE, y + j * BLOCK_SIZE,
                                       colors[CLEAR], TILE_SIZE, TILE_SIZE, BLACK);
                    color_tile(s->tiles[i + j * s->width],
                               x + i * BLOCK_SIZE, y + j * BLOCK_SIZE);
               }
}

// And now draw things on the board.
void
draw_tile(const byte color, const byte x, const byte y) {
     color_tile(color, BOARD_X + x * BLOCK_SIZE, BOARD_Y + y * BLOCK_SIZE);
}

void
draw_shape(const shape *const s, const byte x, const byte y) {
     color_shape(s, BOARD_X + x * BLOCK_SIZE, BOARD_Y + y * BLOCK_SIZE);
}

void 
draw_score(const byte score) {
     if (score < 10) {
          arduboy.write(score + '0');
     } else {
          draw_score(score / 10);
          arduboy.write(score % 10 + '0');
     }
}

void
draw_board(void) {
     arduboy.clear();
     for (byte y = 0; y < BOARD_SIZE; y += 1)
          for (byte x = 0; x < BOARD_SIZE; x += 1)
               draw_tile(game.board[x][y], x, y);
}

void
draw_msg(const char *const msg, byte top = 18) {
     arduboy.setCursor(0, top);
     arduboy.print(msg);
}

void
draw_menu(void) {
     draw_msg(prompt);
     arduboy.print(F("\n\n\33 "));
     arduboy.print(selected == HELP ? F(" Help ") :
                   selected == RESUME ? F("Resume") :
                   selected == KEYS ? F(" Keys ") :
                   selected == NEW ? F(" New  ") :
                   F("BUG!"));
     arduboy.print(F(" \32"));
}

byte
draw_game(void) {
     byte can_place = 0;
     const shape *const s = &shapes[game.waiting[selected]];
     byte red, green, blue;

     red = green = blue = RGB_LED_OFF;

     draw_board();
     arduboy.print(F("Score\n"));
     arduboy.print(game.score);
     arduboy.setCursor(0, 44); 
     arduboy.print(F("High\n"));
     arduboy.print(game.high);
     switch (state) {
     case INIT:
          arduboy.fillRect(0, 0, BOARD_X - 1, 64, 0);
          arduboy.setCursor(0, 16);
          draw_msg("1010 v0.96\n  by mwm\n\nPress A");
          break;
     case DONE:
          blue = BLUE_LED_ON;
          // Fall through
     case MENU:
          draw_menu();
          break;
     case PAGING:
          arduboy.clear();
          strcpy_P(help_buffer, (char *)pgm_read_word(&(help[selected])));
          draw_msg(help_buffer, 0);
          if (selected) {
               arduboy.setCursor(0, 0);
               arduboy.write('\33');
          }
          if (selected < sizeof(help) / sizeof(char *) - 1) {
               arduboy.setCursor(120, 0);
               arduboy.write('\32');
          }
          break;
     default:
          for (byte i = 0; i < WAIT_LEN; i += 1) {
               arduboy.setCursor(BLOCK_SIZE, (i + 3) * BLOCK_SIZE);
               arduboy.write(i == selected ? '+' : game.waiting[i] < 0 ? ' ' : '-');
          }
          color_shape(s, 32 - s->width * BLOCK_SIZE / 2,
                      27 - s->height * BLOCK_SIZE / 2);
          if (state == MOVING) {
               if ((can_place = shape_clear(s, game.x, game.y)))
                    green = GREEN_LED_ON;
               else
                    red = RED_LED_ON;
                    
               draw_shape(s, game.x, game.y);
          }
     }
     arduboy.setRGBled(red, green, blue);
     arduboy.display();
     return can_place;
}


/* Arduino API */
const int store = 266;	// 10 (base 256) + 10. Because why not?
const int saved_game = store + 2;
const int flagbyte = 10;

void
setup(void) {
     arduboy.begin();

     if (EEPROM.read(store) != flagbyte || EEPROM.read(store + 1) != flagbyte) {
          // No saved game
          reset_game();
          game.high = 0;
          selected = HELP;
          EEPROM.put(saved_game, game);
          EEPROM.write(store, flagbyte);
          EEPROM.write(store + 1, flagbyte);
     } else {
          EEPROM.get(saved_game, game);
          selected = NEW;
          prompt = GAMEOVER;
          for (byte i = 0; i < WAIT_LEN; i += 1) {
               if (game.waiting[i] >= 0) {
                    selected = RESUME;
                    prompt = PAUSED;
                    break;
               }
          }
     }
     state = INIT;
}

void
loop(void) {
     const shape *const s = &shapes[game.waiting[selected]];
     const byte can_place = draw_game();

     arduboy.pollButtons();
     if (arduboy.justPressed(B_BUTTON) && (state == SELECTING || state == MOVING)) {
          prompt = PAUSED;
          state = MENU;
          selected = RESUME;
          return;
     }

     switch (state) {
     case INIT:
          if (arduboy.justPressed(A_BUTTON)) {
               arduboy.initRandomSeed();
               state = MENU;
          }
          break;
     case DONE:
          for (byte i = 0; i < WAIT_LEN; i += 1)
               game.waiting[i] = -1;
          // And fall through...
     case MENU:
          if (arduboy.justPressed(A_BUTTON)) {
               do_menu();
          }
          do {
               if (arduboy.justPressed(LEFT_BUTTON)) {
                    if (selected)
                         selected -= 1;
                    else
                         selected = MENU_LAST - 1;
               } else if (arduboy.justPressed(RIGHT_BUTTON)) {
                    selected = (selected + 1) % MENU_LAST;
               }
          } while (prompt == GAMEOVER && selected == RESUME);
          break;
     case PAGING:
          if (arduboy.justPressed(LEFT_BUTTON) && selected)
              selected -= 1;
          else if (arduboy.justPressed(RIGHT_BUTTON)
                   && selected < sizeof(help) / sizeof(char *) - 1)
               selected += 1;
          else if (arduboy.justPressed(UP_BUTTON)) {
               state = prompt == PAUSED ? MENU : DONE;
               selected = HELP;
          }
          break;
     case SELECTING:
          if (arduboy.justPressed(UP_BUTTON))
               do {
                    if (selected)
                         selected -= 1;
                    else
                         selected = WAIT_LEN - 1;
               } while (game.waiting[selected] < 0);
          else if (arduboy.justPressed(DOWN_BUTTON))
               do
                    selected = (selected + 1) % WAIT_LEN;
               while (game.waiting[selected] < 0);
          else if (arduboy.justPressed(RIGHT_BUTTON)) {
               game.x = game.y = 0;
               state = MOVING;
          } else if (arduboy.justPressed(A_BUTTON)) {
               arduboy.setRGBled(RED_LED_ON, RGB_LED_OFF, RGB_LED_OFF);
               if (find_clear(s))
                    state = MOVING;
          }
          break;
     case MOVING:
          if (arduboy.justPressed(A_BUTTON) && can_place)
               make_move();
          else if (arduboy.pressed(UP_BUTTON) && (game.y > 0))
               game.y -= 1;
          else if (arduboy.pressed(DOWN_BUTTON)
                   && (game.y < BOARD_SIZE - s->height))
               game.y += 1;
          else if (arduboy.pressed(RIGHT_BUTTON)
                   && (game.x < BOARD_SIZE - s->width))
               game.x += 1;
          else if (arduboy.pressed(LEFT_BUTTON)) {
               if (game.x > 0)
                    game.x -= 1;
               else
                    state = SELECTING;
          }
          break;
     }

     EEPROM.put(saved_game, game);
     if (state == MOVING) {
          unsigned long end = millis() + game.delay;
          // Used pressed as the button just exists the loop.
          while (millis() < end)
               if (arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON))
                    break;
     }
}
