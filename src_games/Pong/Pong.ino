#include <Arduino.h>
#include "os_libs.h"

// --- Game state ---
static int16_t ballX   = 64, ballY   = 32;
static int8_t  ballDX  = 2,  ballDY  = 1;
static int16_t paddleY = 24;
static uint8_t score   = 0;
static bool    playing = false;

static const int16_t PADDLE_H = 16;
static const int16_t PADDLE_X = 120;

// --- Helpers ---

static void drawStartScreen()
{
    gfx::clear();
    const char *menu[] = {"Start Game", "Quit"};
    gfx::drawMenu(14, 22, menu, 2, 0, 14);
    gfx::display();
}

static void resetGame()
{
    ballX   = 64;
    ballY   = 32;
    ballDX  = 2;
    ballDY  = 1;
    paddleY = 24;
    score   = 0;
}

static void updateGame()
{
    ballX += ballDX;
    ballY += ballDY;

    if (ballY <= 1)  { ballY = 1;  ballDY = -ballDY; }
    if (ballY >= 62) { ballY = 62; ballDY = -ballDY; }
    if (ballX <= 1)  { ballX = 1;  ballDX = -ballDX; }

    if (ballX >= PADDLE_X - 2 && ballX <= PADDLE_X + 1 &&
        ballY >= paddleY && ballY <= paddleY + PADDLE_H) {
        ballX = PADDLE_X - 2;
        ballDX = -ballDX;
        score++;
        beep::tone(880, 20);  // short high beep on paddle hit
    }

    if (ballX >= 127) {
        playing = false;
        beep::tone(220, 300); // low tone on game over
    }

    if (input::pressed(PIN_BTN_UP)   && paddleY > 2)           paddleY -= 2;
    if (input::pressed(PIN_BTN_DOWN) && paddleY < 62 - PADDLE_H) paddleY += 2;
}

static void drawGame()
{
    gfx::clear();

    gfx::drawPixel(ballX, ballY, GFX_WHITE);
    gfx::drawFastVLine(PADDLE_X, paddleY, PADDLE_H, GFX_WHITE);

    // Score bar using drawNumber
    gfx::drawFastHLine(0, 0, GFX_WIDTH, GFX_WHITE);
    gfx::drawNumber(120, GFX_HEIGHT - 7, score, 3);
    gfx::setCursor(2, GFX_HEIGHT - 7);
    gfx::print("B:Quit");

    gfx::display();
}

// --- Entry points ---

void setup()
{
    gfx::init();
    gfx::setTextSize(1);
    gfx::setTextColor(GFX_WHITE);
    input::init();
    ostime::init();
    beep::init();
    rng::init();
    ostime::setFrameRate(40);
}

void loop()
{
    if (!ostime::nextFrame()) return;
    input::update();

    if (!playing) {
        drawStartScreen();

        if (input::justPressed(PIN_BTN_A)) {
            resetGame();
            playing = true;
        }
        if (input::justPressed(PIN_BTN_B)) {
            NVIC_SystemReset();
        }
        return;
    }

    updateGame();
    drawGame();

    if (input::justPressed(PIN_BTN_B)) {
        playing = false;
    }
}
