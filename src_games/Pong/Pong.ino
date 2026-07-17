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
    gfx::drawFastHLine(0, 9, GFX_WIDTH, GFX_WHITE);
    gfx::setCursor(20, 1);
    gfx::print("PONG");

    gfx::setCursor(14, 22);
    gfx::print("UP / DOWN: move");
    gfx::setCursor(14, 32);
    gfx::print("A: start game");
    gfx::setCursor(14, 42);
    gfx::print("B: quit");

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
    // Update ball position
    ballX += ballDX;
    ballY += ballDY;

    // Top/bottom bounce
    if (ballY <= 1)  { ballY = 1;  ballDY = -ballDY; }
    if (ballY >= 62) { ballY = 62; ballDY = -ballDY; }

    // Left wall bounce
    if (ballX <= 1)  { ballX = 1;  ballDX = -ballDX; }

    // Paddle hit
    if (ballX >= PADDLE_X - 2 && ballX <= PADDLE_X + 1 &&
        ballY >= paddleY && ballY <= paddleY + PADDLE_H) {
        ballX = PADDLE_X - 2;
        ballDX = -ballDX;
        score++;
    }

    // Right wall miss — game over
    if (ballX >= 127) {
        playing = false;
        beep::beep_error();
    }

    // Player input
    if (input::pressed(PIN_BTN_UP)   && paddleY > 2)           paddleY -= 2;
    if (input::pressed(PIN_BTN_DOWN) && paddleY < 62 - PADDLE_H) paddleY += 2;
}

static void drawGame()
{
    gfx::clear();

    // Ball and paddle
    gfx::drawPixel(ballX, ballY, GFX_WHITE);
    gfx::drawFastVLine(PADDLE_X, paddleY, PADDLE_H, GFX_WHITE);

    // Score bar
    gfx::drawFastHLine(0, 0, GFX_WIDTH, GFX_WHITE);
    gfx::setCursor(2, GFX_HEIGHT - 7);
    gfx::print("Score:");
    gfx::print((int16_t)score);
    gfx::setCursor(90, GFX_HEIGHT - 7);
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
}

void loop()
{
    input::update();

    if (!playing) {
        drawStartScreen();

        if (input::justPressed(PIN_BTN_A)) {
            resetGame();
            playing = true;
        }
        if (input::justPressed(PIN_BTN_B)) {
            // Exit — restart MCU to return to bootloader
            NVIC_SystemReset();
        }
        ostime::delay_ms(40);
        return;
    }

    updateGame();
    drawGame();

    if (input::justPressed(PIN_BTN_B)) {
        playing = false;
    }

    ostime::delay_ms(25);
}
