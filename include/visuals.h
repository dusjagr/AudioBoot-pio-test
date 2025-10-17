#pragma once
#include <stdint.h>
#include <avr/pgmspace.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"

// Externs provided by the sketch/libs
extern Adafruit_NeoPixel pixels;

// Forward utility from sketch
uint32_t dimColor(uint32_t color, uint8_t scale);
uint32_t Wheel(byte WheelPos);
static inline uint32_t colorFromHeat(uint8_t heat);
void playSound(uint16_t freq, uint16_t dur);

// Visuals API (add more here as we migrate)
void matrixRainbowWaves(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixFire(uint16_t runtime_ms, uint8_t cooling, uint8_t sparking, uint8_t stepDelay_ms);
void matrixTetris(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixDnBDancer(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixKanjiScroll(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixLarsonScanner(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixRain(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixTwinkle(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixWipe(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixShoggoth(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixRainbowZoom(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixBouncingDot(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSpinner(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixPong(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSwitchToggle(uint16_t runtime_ms, uint16_t frameDelay_ms);
void matrixCometSweep(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixPinkSpiral(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixGalagaInvader(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixLightning(uint16_t runtime_ms);
void matrixFlagsShow(uint16_t runtime_ms, uint16_t hold_ms);
void matrixFlagsShowFade(uint16_t runtime_ms, uint16_t hold_ms, uint16_t fade_ms, uint8_t steps);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Visuals Index and Guide
========================================================================================================================
    __     __           _       _ _       
    \ \   / /__ _ __ __| | __ _| (_)_ __  
     \ \ / / _ \ '__/ _` |/ _` | | | '_ \ 
      \ V /  __/ | | (_| | (_| | | | | | |
       \_/ \___|_|  \__,_|\__,_|_|_|_| |_|

  List of available 5x4 matrix effects (runtime_ms, speed params...)

  - matrixRainbowWaves(runtime, stepDelay): Flowing rainbow stripes across the matrix.
  - matrixFire(runtime, cooling, sparking, stepDelay): Fire effect with hotter center and breathing edges.
  - matrixTetris(runtime, stepDelay): Simple falling blocks stack and clear rows.
  - matrixDnBDancer(runtime, stepDelay): Two-frame dancer pulsing to a bass beat.
  - matrixKanjiScroll(runtime, stepDelay): Scrolls a 10x4 kanji-like glyph across the 5x4 matrix.
  - matrixLarsonScanner(runtime, stepDelay): Classic Cylon/Knight Rider scanner with tails.
  - matrixRain(runtime, stepDelay): Blue raindrops fall with fading tails.
  - matrixTwinkle(runtime, stepDelay): Soft random color twinkles that fade.
  - matrixWipe(runtime, stepDelay): Wipe a solid color across rows, back and forth.
  - matrixShoggoth(runtime, stepDelay): Amorphous wobbling blob with blinking eyes.
  - matrixRainbowZoom(runtime, stepDelay): Concentric rainbow rings that zoom.
  - matrixBouncingDot(runtime, stepDelay): Single dot bounces with fading trail.
  - matrixSpinner(runtime, stepDelay): Dot runs around the matrix perimeter.
  - matrixPong(runtime, stepDelay): Mini Pong with AI paddles and bouncing ball.
  - matrixSwitchToggle(runtime, frameDelay): Animated switch body with moving lever.
  - matrixCometSweep(runtime, stepDelay): Bright head sweeps all pixels with trail.
  - matrixPinkSpiral(runtime, stepDelay): Pink dot traces an inward spiral path.
  - matrixGalagaInvader(runtime, stepDelay): Classic alien sprite flaps and sweeps.
  - matrixLightning(runtime): Random lightning strikes with glow and flicker.
  - matrixFlagsShow(runtime, hold_ms): Cycle through several 5x4 national flags.
  - matrixFlagsShowFade(runtime, hold_ms, fade_ms, steps): Flags with cross-fades.

  Tip: For very small flash budgets, comment-out heavier effects or guard them with macros.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// ===== Implementations =====

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowWaves
   Flowing rainbow stripes across the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
static inline uint8_t tri8_local(uint8_t v) {
  uint8_t x = v & 0x7F; // 0..127
  x = (x << 1);         // 0..254
  return (v & 0x80) ? (uint8_t)(255 - x) : x;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixShoggoth
   A wobbling pink blob with soft rim and blinking eyes.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixShoggoth(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint16_t t = 0;
  const uint32_t colCore = pixels.Color(255, 60, 140);
  const uint32_t colRim  = pixels.Color(120, 20, 60);
  const uint32_t colEye  = pixels.Color(255, 255, 255);
  const uint8_t W = 5, H = 4;
  uint8_t eyeCount = 2;
  int8_t ex[3] = {1, 3, 2};
  int8_t ey[3] = {1, 2, 1};
  bool eyesOpen = true;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48);
    uint8_t pulse = (uint8_t)(140 + ((t * 9) & 0x3F));
    uint8_t cx = (uint8_t)(1 + ((t >> 3) & 0x03)); if (cx > (W - 2)) cx = (W - 2);
    uint8_t cy = (uint8_t)(1 + ((t >> 4) & 0x01));
    uint8_t radius = (uint8_t)(1 + ((t >> 2) & 0x01));
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int8_t dx = (int8_t)x - (int8_t)cx;
        int8_t dy = (int8_t)y - (int8_t)cy;
        uint8_t dist = (uint8_t)(abs(dx) + abs(dy));
        if (dist <= radius) {
          matrixSet(x, y, dimColor(colCore, pulse));
        } else if (dist == radius + 1) {
          matrixSet(x, y, dimColor(colRim, (uint8_t)(pulse * 2 / 5)));
        } else if (dist == radius + 2) {
          matrixSet(x, y, dimColor(colRim, (uint8_t)(pulse / 6)));
        }
      }
    }
    if ((t & 0x0F) == 0x00) {
      for (uint8_t i = 0; i < eyeCount; i++) {
        int8_t ox = ((int8_t)random(3)) - 1;
        int8_t oy = ((int8_t)random(3)) - 1;
        int8_t nx = (int8_t)cx + ox;
        int8_t ny = (int8_t)cy + oy;
        if (nx >= 0 && nx < (int8_t)W) ex[i] = (uint8_t)nx;
        if (ny >= 0 && ny < (int8_t)H) ey[i] = (uint8_t)ny;
      }
    }
    if ((t & 0x1F) == 0x1F) { eyesOpen = !eyesOpen; }
    if (eyesOpen) {
      for (uint8_t i = 0; i < eyeCount; i++) matrixSet((uint8_t)ex[i], (uint8_t)ey[i], colEye);
    } else {
      for (uint8_t i = 0; i < eyeCount; i++) matrixSet((uint8_t)ex[i], (uint8_t)ey[i], dimColor(colEye, 100));
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowZoom
   Concentric rainbow rings that appear to zoom in/out.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRainbowZoom(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint16_t phase = 0;
  const uint8_t W = 5, H = 4;
  const int16_t CX = 2 * 16 + 8; // 2.5
  const int16_t CY = 1 * 16 + 8; // 1.5
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int16_t fx = (int16_t)x * 16 + 8;
        int16_t fy = (int16_t)y * 16 + 8;
        int16_t dx = (fx - CX);
        int16_t dy = (fy - CY);
        uint16_t manh = (uint16_t)(abs(dx) + abs(dy));
        uint8_t hue = (uint8_t)((manh * 4 + phase) & 0xFF);
        uint32_t col = dimColor(Wheel(hue), 200);
        matrixSet(x, y, col);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    phase += 9;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixBouncingDot
   A single colored dot bounces around with a fading trail.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixBouncingDot(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  int8_t x = 0, y = 0;
  int8_t vx = 1, vy = 1;
  uint32_t start = millis();
  uint16_t frame = 0;
  matrixFill(0);
  pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48);
    uint8_t hue = (frame * 6) & 0xFF;
    matrixSet((uint8_t)x, (uint8_t)y, Wheel(hue));
    pixels.show();
    delay(stepDelay_ms);
    x += vx; y += vy;
    if (x < 0) { x = 0; vx = -vx; }
    if (x >= MATRIX_W) { x = MATRIX_W - 1; vx = -vx; }
    if (y < 0) { y = 0; vy = -vy; }
    if (y >= MATRIX_H) { y = MATRIX_H - 1; vy = -vy; }
    frame++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSpinner
   A rainbow dot runs around the outer edge of the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSpinner(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t pathLen = (MATRIX_W * 2) + (MATRIX_H * 2) - 4;
  uint8_t xs[24], ys[24], idx = 0;
  for (uint8_t x = 0; x < MATRIX_W; x++) { xs[idx] = x; ys[idx] = 0; idx++; }
  for (uint8_t y = 1; y < MATRIX_H; y++) { xs[idx] = MATRIX_W - 1; ys[idx] = y; idx++; }
  for (int8_t x = MATRIX_W - 2; x >= 0; x--) { xs[idx] = (uint8_t)x; ys[idx] = MATRIX_H - 1; idx++; }
  for (int8_t y = MATRIX_H - 2; y > 0; y--) { xs[idx] = 0; ys[idx] = (uint8_t)y; idx++; }
  uint32_t start = millis();
  matrixFill(0); pixels.show();
  uint16_t step = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48);
    uint8_t p = step % pathLen;
    uint32_t c = Wheel((step * 5) & 0xFF);
    matrixSet(xs[p], ys[p], c);
    pixels.show();
    delay(stepDelay_ms);
    step++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixPong
   Mini Pong: AI paddles on the sides, ball bounces with simple spin.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixPong(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t PADDLE_H = 2;
  const uint8_t LEFT_X = 0;
  const uint8_t RIGHT_X = MATRIX_W - 1;
  int8_t ballX = MATRIX_W / 2, ballY = MATRIX_H / 2;
  int8_t vx = 1, vy = 1;
  int8_t lpY = 1, rpY = 1;
  const uint32_t colBall = pixels.Color(255, 200, 220);
  const uint32_t colPaddle = pixels.Color(40, 200, 120);
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFill(0);
    for (uint8_t i = 0; i < PADDLE_H; i++) {
      int8_t ly = lpY + i; if (ly >= 0 && ly < MATRIX_H) matrixSet(LEFT_X, (uint8_t)ly, colPaddle);
      int8_t ry = rpY + i; if (ry >= 0 && ry < MATRIX_H) matrixSet(RIGHT_X, (uint8_t)ry, colPaddle);
    }
    if (ballX >= 0 && ballX < MATRIX_W && ballY >= 0 && ballY < MATRIX_H) matrixSet((uint8_t)ballX, (uint8_t)ballY, colBall);
    pixels.show();
    delay(stepDelay_ms);
    ballX += vx; ballY += vy;
    if (ballY < 0) { ballY = 0; vy = -vy; }
    if (ballY >= MATRIX_H) { ballY = MATRIX_H - 1; vy = -vy; }
    if (vx < 0 && ballX == (LEFT_X + 1)) { if (ballY >= lpY && ballY < (lpY + PADDLE_H)) { vx = 1; vy = ((ballY - lpY) == 0) ? -1 : 1; } }
    if (vx > 0 && ballX == (RIGHT_X - 1)) { if (ballY >= rpY && ballY < (rpY + PADDLE_H)) { vx = -1; vy = ((ballY - rpY) == 0) ? -1 : 1; } }
    if (ballX < 0 || ballX >= MATRIX_W) { ballX = MATRIX_W / 2; ballY = MATRIX_H / 2; vx = (random(2) == 0) ? 1 : -1; vy = (random(2) == 0) ? 1 : -1; }
    int8_t lpCenter = lpY + (PADDLE_H / 2); if (ballY > lpCenter) lpY++; else if (ballY < lpCenter) lpY--; if (lpY < 0) lpY = 0; if (lpY > (MATRIX_H - PADDLE_H)) lpY = MATRIX_H - PADDLE_H;
    int8_t rpCenter = rpY + (PADDLE_H / 2); if (ballY > rpCenter) rpY++; else if (ballY < rpCenter) rpY--; if (rpY < 0) rpY = 0; if (rpY > (MATRIX_H - PADDLE_H)) rpY = MATRIX_H - PADDLE_H;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSwitchToggle
   A small switch body whose lever animates between closed/open.
   Params: runtime_ms, frameDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSwitchToggle(uint16_t runtime_ms, uint16_t frameDelay_ms) {
  const uint8_t F = 4;
  const uint8_t frames[F][20] = {
    {0,1,1,1,0, 0,1,2,1,0, 0,1,1,1,0, 0,0,1,0,0},
    {0,1,1,1,0, 0,1,1,2,0, 0,1,1,1,0, 0,0,1,0,0},
    {0,1,1,1,0, 0,2,1,1,0, 0,1,1,1,0, 0,0,1,0,0},
    {0,1,1,1,0, 2,1,1,1,0, 0,1,1,1,0, 0,0,1,0,0}
  };
  uint32_t start = millis();
  uint8_t idx = 0; int8_t dir = 1;
  const uint32_t body = pixels.Color(80, 80, 80);
  const uint32_t leverOn = pixels.Color(255, 20, 120);
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t v = frames[idx][y * MATRIX_W + x];
        matrixSet(x, y, v == 0 ? 0 : (v == 1 ? body : leverOn));
      }
    }
    pixels.show();
    delay(frameDelay_ms);
    idx += dir;
    if (idx == 0 || idx == (F - 1)) dir = -dir;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixCometSweep
   A bright "comet" head sweeps through all pixels with a soft trail.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixCometSweep(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0); pixels.show();
  uint16_t pos = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(56);
    uint8_t y = (pos / MATRIX_W) % MATRIX_H;
    uint8_t x = pos % MATRIX_W;
    int id = matrixIndex(x, y);
    pixels.setPixelColor(id, Wheel((pos * 7) & 0xFF));
    pixels.show();
    delay(stepDelay_ms);
    pos = (pos + 1) % (MATRIX_W * MATRIX_H);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixPinkSpiral
   A vivid pink dot traces an inward spiral path with a gentle trail.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixPinkSpiral(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint8_t order[W * H][2];
  uint8_t left = 0, right = W - 1, top = 0, bottom = H - 1;
  uint8_t idx = 0;
  while (left <= right && top <= bottom) {
    for (uint8_t x = left; x <= right; x++) { order[idx][0] = x; order[idx][1] = top; idx++; }
    for (uint8_t y = top + 1; y <= bottom; y++) { order[idx][0] = right; order[idx][1] = y; idx++; }
    if (top < bottom) { for (int8_t x = right - 1; x >= (int8_t)left; x--) { order[idx][0] = (uint8_t)x; order[idx][1] = bottom; idx++; } }
    if (left < right) { for (int8_t y = bottom - 1; y > (int8_t)top; y--) { order[idx][0] = left; order[idx][1] = (uint8_t)y; idx++; } }
    left++; right--; top++; bottom--;
  }
  uint32_t start = millis(); uint16_t step = 0;
  matrixFill(0); pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(56);
    uint8_t p = step % (W * H);
    matrixSet(order[p][0], order[p][1], pixels.Color(255, 60, 140));
    pixels.show();
    delay(stepDelay_ms);
    step++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixGalagaInvader
   Classic alien sprite flaps as it sweeps across the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixGalagaInvader(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  static const uint8_t PROGMEM F0[20] = {
    0,1,0,1,0,
    1,1,2,1,1,
    1,1,1,1,1,
    0,1,0,1,0
  };
  static const uint8_t PROGMEM F1[20] = {
    0,1,0,1,0,
    1,1,2,1,1,
    1,0,1,0,1,
    1,1,0,1,1
  };
  const uint8_t* frames[2] = { F0, F1 };
  const uint32_t colBody = pixels.Color(60, 220, 100);
  const uint32_t colEye  = pixels.Color(255, 255, 255);
  uint32_t start = millis(); uint8_t fidx = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (int8_t ox = -4; ox <= (int8_t)W; ox++) {
      matrixFill(0);
      const uint8_t* spr = frames[fidx];
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          uint8_t v = pgm_read_byte(&spr[y * 5 + x]);
          int8_t tx = (int8_t)x + ox; if (tx < 0 || tx >= (int8_t)W) continue;
          if (v == 1) matrixSet((uint8_t)tx, y, colBody); else if (v == 2) matrixSet((uint8_t)tx, y, colEye);
        }
      }
      pixels.show(); delay(stepDelay_ms); fidx ^= 1; if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
    for (int8_t ox = W; ox >= -4; ox--) {
      matrixFill(0);
      const uint8_t* spr = frames[fidx];
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          uint8_t v = pgm_read_byte(&spr[y * 5 + x]);
          int8_t tx = (int8_t)x + ox; if (tx < 0 || tx >= (int8_t)W) continue;
          if (v == 1) matrixSet((uint8_t)tx, y, colBody); else if (v == 2) matrixSet((uint8_t)tx, y, colEye);
        }
      }
      pixels.show(); delay(stepDelay_ms); fidx ^= 1; if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixLightning
   Random lightning strikes with blue glow, flicker and occasional clicks.
   Params: runtime_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixLightning(uint16_t runtime_ms) {
  uint32_t start = millis(); randomSeed(millis());
  const uint8_t W = 5, H = 4;
  auto clampi = [](int v, int lo, int hi) -> int { return (v < lo) ? lo : (v > hi ? hi : v); };
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint16_t darkMs = 120 + (random(280));
    matrixFill(0); pixels.show(); delay(darkMs);
    uint8_t flashes = 1 + (random(3));
    for (uint8_t f = 0; f < flashes; f++) {
      int x = random(W), y = 0;
      uint8_t steps = H + random(2);
      matrixFill(0);
      for (uint8_t s = 0; s < steps; s++) {
        matrixSet((uint8_t)x, (uint8_t)y, pixels.Color(255, 255, 255));
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = clampi(x + dx, 0, W - 1);
            int ny = clampi(y + dy, 0, H - 1);
            matrixSet((uint8_t)nx, (uint8_t)ny, pixels.Color(80, 120, 255));
          }
        }
        pixels.show(); delay(12 + random(16));
        x += (int8_t)(random(3)) - 1; y += 1; if (y >= H) y = H - 1; x = clampi(x, 0, W - 1);
      }
      for (uint8_t k = 0; k < 3; k++) {
        matrixFade(200);
        for (uint8_t i = 0; i < 3; i++) { int rx = random(W); int ry = random(H); matrixSet((uint8_t)rx, (uint8_t)ry, pixels.Color(255, 255, 255)); }
        pixels.show(); delay(20 + random(20));
      }
      for (uint8_t d = 0; d < 4; d++) { matrixFade(120); pixels.show(); delay(18); }
      if (random(3) == 0) { playSound(80 + random(60), 20 + random(30)); }
    }
  }
}

// Flags engine (5x4)
inline void displayFlag(const uint32_t flag[20]) {
  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      uint32_t c = pgm_read_dword(&flag[y * 5 + x]);
      uint8_t r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;
      matrixSet(x, y, pixels.Color(r, g, b));
    }
  }
  pixels.show();
}

static const uint32_t COL_BLACK  = 0;
static const uint32_t COL_WHITE  = 0xFFFFFF;
static const uint32_t COL_RED    = 0xFF0000;
static const uint32_t COL_BLUE   = 0x0000FF;
static const uint32_t COL_YELLOW = 0xFFFF00;
static const uint32_t COL_GREEN  = 0x00FF00;
static const uint32_t COL_ORANGE = 0xFF8C00;

static const uint32_t FLAG_DE[20] PROGMEM = {
  COL_BLACK, COL_BLACK, COL_BLACK, COL_BLACK, COL_BLACK,
  COL_RED,   COL_RED,   COL_RED,   COL_RED,   COL_RED,
  COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,
  COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE
};
static const uint32_t FLAG_FR[20] PROGMEM = {
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED
};
static const uint32_t FLAG_IT[20] PROGMEM = {
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED
};
static const uint32_t FLAG_CH[20] PROGMEM = {
  COL_RED, COL_RED, COL_WHITE, COL_RED, COL_RED,
  COL_RED, COL_WHITE, COL_WHITE, COL_WHITE, COL_RED,
  COL_RED, COL_RED, COL_WHITE, COL_RED, COL_RED,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED
};
static const uint32_t FLAG_SE[20] PROGMEM = {
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW,
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE
};
static const uint32_t FLAG_UA[20] PROGMEM = {
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW
};
static const uint32_t FLAG_PL[20] PROGMEM = {
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED
};
static const uint32_t FLAG_NL[20] PROGMEM = {
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED,
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE
};

inline void matrixFlagsShow(uint16_t runtime_ms, uint16_t hold_ms) {
  const uint8_t N = 8; const uint32_t* flags[N] = { FLAG_DE, FLAG_FR, FLAG_IT, FLAG_CH, FLAG_SE, FLAG_UA, FLAG_PL, FLAG_NL };
  uint32_t start = millis(); uint8_t idx = 0; while ((uint16_t)(millis() - start) < runtime_ms) { displayFlag(flags[idx]); delay(hold_ms); idx = (idx + 1) % N; }
}

inline void fadeFlags(const uint32_t* a, const uint32_t* b, uint16_t fade_ms, uint8_t steps) {
  if (steps == 0 || fade_ms == 0) return; uint16_t stepDelay = fade_ms / steps;
  for (uint8_t s = 1; s <= steps; s++) {
    uint8_t w2 = s; uint8_t w1 = steps - s;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t i = y * 5 + x; uint32_t c1 = pgm_read_dword(&a[i]); uint32_t c2 = pgm_read_dword(&b[i]);
        uint8_t r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF; uint8_t r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
        uint8_t r = (uint16_t)r1 * w1 / steps + (uint16_t)r2 * w2 / steps; uint8_t g = (uint16_t)g1 * w1 / steps + (uint16_t)g2 * w2 / steps; uint8_t bch = (uint16_t)b1 * w1 / steps + (uint16_t)b2 * w2 / steps;
        matrixSet(x, y, pixels.Color(r, g, bch));
      }
    }
    pixels.show(); delay(stepDelay);
  }
}

inline void matrixFlagsShowFade(uint16_t runtime_ms, uint16_t hold_ms, uint16_t fade_ms, uint8_t steps) {
  const uint8_t N = 8; const uint32_t* flags[N] = { FLAG_DE, FLAG_FR, FLAG_IT, FLAG_CH, FLAG_SE, FLAG_UA, FLAG_PL, FLAG_NL };
  uint32_t start = millis(); uint8_t idx = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    const uint32_t* cur = flags[idx]; const uint32_t* nxt = flags[(idx + 1) % N];
    displayFlag(cur); delay(hold_ms); fadeFlags(cur, nxt, fade_ms, steps); idx = (idx + 1) % N;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixDnBDancer
   Two-frame dancer that pulses on bass hits with accent colors.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixDnBDancer(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  static const uint8_t PROGMEM P0[20] = {
    0,1,0,1,0,
    0,1,1,1,0,
    1,0,1,0,1,
    0,1,0,1,0
  };
  static const uint8_t PROGMEM P1[20] = {
    0,1,0,1,0,
    1,1,1,1,1,
    0,1,0,1,0,
    1,0,1,0,1
  };
  const uint8_t* frames[2] = { P0, P1 };
  uint32_t start = millis();
  uint16_t beat = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    bool bass = ((beat & 0x07) == 0);
    if (bass) {
      for (uint8_t y = 0; y < H; y++) for (uint8_t x = 0; x < W; x++) matrixSet(x, y, dimColor(pixels.Color(255,60,140), 80));
    } else {
      matrixFade(64);
    }
    uint8_t f = (beat & 0x01);
    const uint8_t* spr = frames[f];
    uint32_t colBody = pixels.Color(255, 80, 160);
    uint32_t colAcc  = bass ? pixels.Color(40, 200, 200) : pixels.Color(200, 120, 220);
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t v = pgm_read_byte(&spr[y * W + x]);
        if (v == 1) matrixSet(x, y, colBody);
        else if (v == 2) matrixSet(x, y, colAcc);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    beat++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixKanjiScroll
   Scrolls a 10x4 kanji-like glyph across the 5x4 display.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixKanjiScroll(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  const uint8_t GW = 10;
  static const uint8_t PROGMEM glyph[40] = {
    0,1,0,0,1, 0,1,0,0,0,
    1,0,1,0,1, 0,1,0,0,0,
    0,1,0,1,0, 1,0,1,0,0,
    0,0,1,0,0, 1,0,1,0,0
  };
  const uint32_t colOn  = pixels.Color(255, 60, 140);
  uint32_t start = millis();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (int8_t ox = W; ox >= -(int8_t)GW; ox--) {
      matrixFill(0);
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t gx = 0; gx < GW; gx++) {
          uint8_t v = pgm_read_byte(&glyph[y * GW + gx]);
          int8_t tx = (int8_t)gx + ox;
          if (v && tx >= 0 && tx < (int8_t)W) {
            matrixSet((uint8_t)tx, y, colOn);
          }
        }
      }
      pixels.show();
      delay(stepDelay_ms);
      if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixLarsonScanner
   Classic Cylon/Knight Rider scanner with symmetric fading tails.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixLarsonScanner(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  int8_t pos = 0;
  int8_t dir = 1;
  uint8_t y = (MATRIX_H > 2) ? 2 : 1;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(40);
    uint32_t head = pixels.Color(255, 0, 0);
    uint32_t tail1 = dimColor(head, 140);
    uint32_t tail2 = dimColor(head, 60);
    int8_t yUp = ((int8_t)y) - 1;
    int8_t yDown = ((int8_t)y) + 1;
    uint32_t headAdj = dimColor(head, 80);
    uint8_t oppX = (uint8_t)(MATRIX_W - 1 - pos);
    matrixSet((uint8_t)pos, y, head);
    if (pos - 1 >= 0) matrixSet((uint8_t)(pos - 1), y, tail1);
    if (pos + 1 < MATRIX_W) matrixSet((uint8_t)(pos + 1), y, tail1);
    if (pos - 2 >= 0) matrixSet((uint8_t)(pos - 2), y, tail2);
    if (pos + 2 < MATRIX_W) matrixSet((uint8_t)(pos + 2), y, tail2);
    if (yUp >= 0) { matrixSet(oppX, (uint8_t)yUp, headAdj); }
    if (yDown < (int8_t)MATRIX_H) { matrixSet(oppX, (uint8_t)yDown, headAdj); }
    pixels.show();
    delay(stepDelay_ms);
    pos += dir;
    if (pos <= 0) { pos = 0; dir = 1; }
    if (pos >= (MATRIX_W - 1)) { pos = MATRIX_W - 1; dir = -1; }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRain
   Blue droplets spawn at the top and fall down, leaving fading trails.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRain(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(64);
    if ((millis() & 0x03) == 0) {
      uint8_t x = (uint8_t)random(MATRIX_W);
      matrixSet(x, 0, pixels.Color(20, 60, 150));
    }
    for (int8_t y = MATRIX_H - 1; y >= 0; y--) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        int toIdx = matrixIndex(x, y);
        int fromIdx = (y == 0) ? -1 : matrixIndex(x, y - 1);
        uint32_t c = (fromIdx >= 0) ? pixels.getPixelColor(fromIdx) : 0;
        pixels.setPixelColor(toIdx, c);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixTwinkle
   Soft, random multicolor twinkles that fade away gradually.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixTwinkle(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(40);
    for (uint8_t i = 0; i < 2; i++) {
      uint8_t x = (uint8_t)random(MATRIX_W);
      uint8_t y = (uint8_t)random(MATRIX_H);
      uint8_t hue = (uint8_t)random(256);
      uint32_t c = dimColor(Wheel(hue), 160);
      matrixSet(x, y, c);
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixWipe
   Wipes a solid color across rows left-to-right and back again.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixWipe(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  uint8_t phase = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint8_t hue = (phase * 8);
    uint32_t c = Wheel(hue);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      if (y % 2 == 0) {
        for (uint8_t x = 0; x < MATRIX_W; x++) { matrixSet(x, y, c); pixels.show(); delay(stepDelay_ms); }
      } else {
        for (int8_t x = MATRIX_W - 1; x >= 0; x--) { matrixSet((uint8_t)x, y, c); pixels.show(); delay(stepDelay_ms); }
      }
    }
    matrixFill(0);
    pixels.show();
    phase++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixTetris
   Simple falling blocks stack up; full rows are cleared.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixTetris(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  uint8_t field[W * H];
  for (uint8_t i = 0; i < W * H; i++) field[i] = 0;

  auto idx = [&](int8_t x, int8_t y) -> int { return y * W + x; };
  auto occupied = [&](int8_t x, int8_t y) -> bool {
    if (x < 0 || x >= (int8_t)W || y < 0 || y >= (int8_t)H) return true; // out of bounds = solid
    return field[idx(x, y)] != 0;
  };

  // 3 simple pieces (no rotation), each as 3-4 blocks
  const int8_t P0[4][2] = {{0,0},{1,0},{0,1},{1,1}};          // square
  const int8_t P1[3][2] = {{0,0},{1,0},{2,0}};                // bar
  const int8_t P2[4][2] = {{0,0},{0,1},{1,1},{2,1}};          // L-ish

  uint32_t start = millis();
  uint8_t hueBase = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint8_t kind = (uint8_t)random(3);
    int8_t px = (kind == 1) ? (int8_t)random(0, (int8_t)(W - 2)) : (int8_t)random(0, (int8_t)(W - 1));
    int8_t py = H; // start just above top
    uint8_t hue = hueBase;

    bool locked = false;
    while (!locked && (uint16_t)(millis() - start) < runtime_ms) {
      int8_t ny = py - 1;
      bool hit = false;
      if (kind == 0) { for (uint8_t i = 0; i < 4; i++) { if (occupied(px + P0[i][0], ny + P0[i][1])) { hit = true; break; } } }
      else if (kind == 1) { for (uint8_t i = 0; i < 3; i++) { if (occupied(px + P1[i][0], ny + P1[i][1])) { hit = true; break; } } }
      else { for (uint8_t i = 0; i < 4; i++) { if (occupied(px + P2[i][0], ny + P2[i][1])) { hit = true; break; } } }

      if (hit) {
        if (kind == 0) { for (uint8_t i = 0; i < 4; i++) field[idx(px + P0[i][0], py + P0[i][1])] = 1; }
        else if (kind == 1) { for (uint8_t i = 0; i < 3; i++) field[idx(px + P1[i][0], py + P1[i][1])] = 1; }
        else { for (uint8_t i = 0; i < 4; i++) field[idx(px + P2[i][0], py + P2[i][1])] = 1; }
        locked = true;
        // Clear any full rows
        for (uint8_t y = 0; y < H; y++) {
          bool full = true;
          for (uint8_t x = 0; x < W; x++) { if (!field[idx(x, y)]) { full = false; break; } }
          if (full) {
            for (int yy = y; yy < (int)H - 1; yy++) {
              for (uint8_t x = 0; x < W; x++) field[idx(x, yy)] = field[idx(x, yy + 1)];
            }
            for (uint8_t x = 0; x < W; x++) field[idx(x, H - 1)] = 0;
            y--; // recheck
          }
        }
        break;
      } else {
        py = ny;
      }

      // Render field + active piece
      matrixFill(0);
      for (uint8_t y = 0; y < H; y++) for (uint8_t x = 0; x < W; x++) if (field[idx(x, y)]) matrixSet(x, y, dimColor(pixels.Color(255,160,80), 180));
      uint32_t col = Wheel(hue);
      if (kind == 0) { for (uint8_t i = 0; i < 4; i++) { int8_t ax = px + P0[i][0], ay = py + P0[i][1]; if (ay >= 0 && ay < (int8_t)H) matrixSet(ax, ay, col); } }
      else if (kind == 1) { for (uint8_t i = 0; i < 3; i++) { int8_t ax = px + P1[i][0], ay = py + P1[i][1]; if (ay >= 0 && ay < (int8_t)H) matrixSet(ax, ay, col); } }
      else { for (uint8_t i = 0; i < 4; i++) { int8_t ax = px + P2[i][0], ay = py + P2[i][1]; if (ay >= 0 && ay < (int8_t)H) matrixSet(ax, ay, col); } }
      pixels.show();
      delay(stepDelay_ms);
      hue += 7;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixRainbowWaves
   Flowing rainbow stripes across the matrix.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixRainbowWaves(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;
  uint32_t start = millis();
  uint8_t t = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t phase = (uint8_t)(x * 32 + y * 48 + t);
        uint8_t wv = tri8_local(phase); // 0..255 wave
        uint8_t hue = (uint8_t)(phase + (wv >> 2));
        uint32_t col = dimColor(Wheel(hue), 200);
        matrixSet(x, y, col);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    t += 5;
  }
}

// Map a heat value (0..255) to a fire-like color (black -> red -> yellow -> white)
static inline uint32_t colorFromHeat(uint8_t heat) {
    uint8_t t192 = (uint16_t)heat * 191 / 255;   // 0..191
    uint8_t heatramp = (t192 & 63) << 2;         // 0..252 in steps of 4
    if (t192 > 128) {
      return pixels.Color(255, 255, heatramp);   // white-ish
    } else if (t192 > 64) {
      return pixels.Color(255, heatramp, 0);     // yellow->red
    } else {
      return pixels.Color(heatramp, 0, 0);       // dark red
    }
  }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFire
   Classic Fire2012-inspired effect adapted to 5x4 with hotter center.
   Params: runtime_ms, cooling, sparking, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFire(uint16_t runtime_ms, uint8_t cooling, uint8_t sparking, uint8_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t FW = MATRIX_W; // matrix width
  const uint8_t FH = MATRIX_H; // matrix height
  uint8_t heat[FW * FH];
  for (uint8_t i = 0; i < FW * FH; i++) heat[i] = 0;
  auto idxAt = [&](uint8_t x, uint8_t y) -> int { return y * FW + x; };
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Cool
    for (uint8_t x = 0; x < FW; x++) {
      for (uint8_t y = 0; y < FH; y++) {
        int i = idxAt(x, y);
        uint8_t cool = (uint8_t)random((cooling / FH) + 2);
        heat[i] = (heat[i] > cool) ? (heat[i] - cool) : 0;
      }
    }
    // Diffuse upward (include row 1 so it gets heat from row 0)
    for (int y = FH - 1; y >= 1; y--) {
      for (uint8_t x = 0; x < FW; x++) {
        int i = idxAt(x, y);
        int below = idxAt(x, y - 1);
        int below2 = idxAt(x, (y >= 2) ? (y - 2) : 0); // clamp for y==1
        heat[i] = (uint8_t)(((int)heat[below] + (int)heat[below] + (int)heat[below2]) / 3);
      }
    }
    // New sparks at the bottom
    for (uint8_t x = 0; x < FW; x++) {
      if ((uint8_t)random(255) < sparking) {
        int i = idxAt(x, 0);
        heat[i] = (uint8_t)min(255, heat[i] + (int)random(160, 255));
      }
    }
    // Breathing edge scaling
    uint8_t tb = (uint8_t)((millis() >> 6) & 0xFF);
    uint8_t tri = (tb & 0x80) ? (uint8_t)(255 - ((tb & 0x7F) << 1)) : (uint8_t)(((tb & 0x7F) << 1));
    int16_t delta = (int16_t)tri - 128; // -128..127
    int16_t s1i = 200 + (delta * 20) / 128; // near edge 180..220
    int16_t s2i = 110 + (delta * 25) / 128; // far edge 85..135
    uint8_t s1 = (s1i < 0) ? 0 : (s1i > 255 ? 255 : (uint8_t)s1i);
    uint8_t s2 = (s2i < 0) ? 0 : (s2i > 255 ? 255 : (uint8_t)s2i);

    // Draw with hotter center
    for (uint8_t y = 0; y < FH; y++) {
      for (uint8_t x = 0; x < FW; x++) {
        int i = idxAt(x, y);
        uint8_t center = FW / 2; // integer center
        uint8_t d = (x > center) ? (x - center) : (center - x);
        uint8_t scale = (d == 0) ? 255 : (d == 1 ? s1 : s2);
        uint8_t adj = (uint16_t)heat[i] * scale / 255;
        matrixSet(x, y, colorFromHeat(adj));
      }
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}
