// Ensure dependencies are available before any implementations that use MATRIX_W/H
#include <stdint.h>
#include <avr/pgmspace.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"

// [Note for contributors]
// Implementations of visuals are placed below in the "===== Implementations =====" section.
// When adding a new effect:
//  1) Declare its prototype in the Visuals API block below (void myVisual(...);
//  2) Implement it in the Implementations section alongside the others (group by theme if possible).
//  3) Add an entry in src/AetzLampliu_mini.ino's Visual Selector and (optionally) set it as default.
//  4) Keep RAM tight for ATtiny85; prefer small local arrays and reuse helpers (matrixFade, dimColor, etc.).

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
void matrixFiveEightSeam(uint16_t runtime_ms, uint16_t bpm8, uint8_t col);
void matrixCoteAzur(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixSunsetPickleSun(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixLarsonScannerDual(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixExplosion(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixDigitalRain(uint16_t runtime_ms, uint16_t stepDelay_ms);
void matrixWaterfall(uint16_t runtime_ms, uint16_t stepDelay_ms);
static inline void matrixNightStreet2000(uint16_t runtime_ms, uint16_t stepDelay_ms);

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
  - matrixDigitalRain(runtime, stepDelay): Matrix movie-style green code rain adapted to 5x4.

  Tip: For very small flash budgets, comment-out heavier effects or guard them with macros.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// ===== Implementations =====

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixFiveEightSeam
   Illuminate one matrix column as a 5/8 rhythm (5 eighths per bar) with non-repeating accents.
   - Strong beats (group starts) are bright; other beats are dim.
   - Accents vary per bar using a tiny LFSR and rotating start offset so it practically never repeats.
   Params: runtime_ms, bpm8 (eighth-notes per minute), col (0..MATRIX_W-1)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixFiveEightSeam(uint16_t runtime_ms, uint16_t bpm8, uint8_t col) {
  if (col >= MATRIX_W) col = MATRIX_W - 1;
  uint32_t start = millis();
  if (bpm8 < 10) bpm8 = 10; // safety lower bound
  uint16_t stepMs = (uint16_t)(60000UL / bpm8); // one 8th-note in ms

  // LFSR for pseudo-random accent variation (16-bit, taps 16,14,13,11)
  uint16_t lfsr = (uint16_t)(millis() ^ 0xBEEF);
  auto nextLfsr = [&]() {
    uint16_t bit = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (bit << 15));
    return lfsr;
  };

  // Accent pattern per bar built from either 3+2 or 2+3 grouping; rotated by offset
  uint8_t bar = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Choose grouping: 0 => [3,2], 1 => [2,3]
    bool two_three = (nextLfsr() & 1);
    uint8_t accents[5] = {0,0,0,0,0};
    if (two_three) { accents[0] = 1; accents[2] = 1; accents[3] = 1; } // 2+3 strong at 0 and 2 (and optional extra at 3)
    else            { accents[0] = 1; accents[3] = 1; }                 // 3+2 strong at 0 and 3
    // Rotate start offset by a value derived from LFSR and bar index
    uint8_t rot = (uint8_t)((nextLfsr() + bar) % 5);
    auto accAt = [&](uint8_t i) -> uint8_t { return accents[(i + rot) % 5]; };

    for (uint8_t i = 0; i < 5; i++) {
      // Compute on-time and off-time fractions for visibility
      uint16_t onMs = (uint16_t)(stepMs * 3 / 5);   // 60% on for richer color
      uint16_t offMs = (uint16_t)(stepMs - onMs);   // 40% off
      uint16_t mainMs = (uint16_t)(onMs * 2 / 3);   // primary hit duration
      uint16_t echoMs = (uint16_t)(onMs - mainMs);  // echo duration

      // Dynamic hue per beat, slowly rotating per bar for variety
      uint8_t baseHue = (uint8_t)((bar * 37 + i * 21) & 0xFF);
      bool strong = accAt(i) != 0;
      uint8_t mainHue = strong ? baseHue : (uint8_t)(baseHue + 28);
      uint8_t mainLevel = strong ? 255 : 150;   // strong beats brighter

      // Draw main seam column with a slight vertical hue gradient
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        uint8_t yHue = (uint8_t)(mainHue + y * 10);
        uint32_t c = dimColor(Wheel(yHue), mainLevel);
        matrixSet(col, y, c);
      }


      // Light adjacent columns with falloff for a wider, more interesting look
      if (col > 0) {
        uint8_t leftLevel = (uint8_t)(mainLevel / 3);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(mainHue + 8 + y * 10);
          matrixSet((uint8_t)(col - 1), y, dimColor(Wheel(yHue), leftLevel));
        }
      }
      if (col + 1 < MATRIX_W) {
        uint8_t rightLevel = (uint8_t)(mainLevel / 3);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(mainHue - 8 + y * 10);
          matrixSet((uint8_t)(col + 1), y, dimColor(Wheel(yHue), rightLevel));
        }
      }
      // Occasional sparkle on strong beats
      if (strong && ((nextLfsr() & 0x03) == 0)) {
        uint8_t sy = (uint8_t)(nextLfsr() % MATRIX_H);
        matrixSet(col, sy, pixels.Color(255, 255, 255));
      }
      pixels.show();
      delay(mainMs);

      // Echo on mirrored seam in second half of the beat
      uint8_t mirrorCol = (uint8_t)(MATRIX_W - 1 - col);
      uint8_t echoHue = (uint8_t)(mainHue + 96);           // complementary-ish
      uint8_t echoLevel = strong ? 160 : 220;              // weak beats respond stronger
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        uint8_t yHue = (uint8_t)(echoHue + y * 12);
        matrixSet(mirrorCol, y, dimColor(Wheel(yHue), echoLevel));
      }
      if (mirrorCol > 0) {
        uint8_t ml = (uint8_t)(echoLevel / 3);
        uint8_t mcL = (uint8_t)(mirrorCol - 1);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(echoHue + 6 + y * 12);
          matrixSet(mcL, y, dimColor(Wheel(yHue), ml));
        }
      }
      if (mirrorCol + 1 < MATRIX_W) {
        uint8_t mr = (uint8_t)(echoLevel / 3);
        uint8_t mcR = (uint8_t)(mirrorCol + 1);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
          uint8_t yHue = (uint8_t)(echoHue - 6 + y * 12);
          matrixSet(mcR, y, dimColor(Wheel(yHue), mr));
        }
      }
      pixels.show();
      delay(echoMs);

      // Off phase: gentle fade to leave trailing glow
      matrixFade(140);
      pixels.show();
      delay(offMs);

      if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
    bar++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixDigitalRain
   Matrix movie-style green code rain. On a tiny 5x4, each column spawns a bright green head
   that falls with variable speed, leaving a trailing fade. Heads respawn pseudo-randomly.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixDigitalRain(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;

  // Small LFSR for pseudo-randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0x1234);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Per-column head position (0..H-1) or 0xFF when inactive
  uint8_t headY[8];
  // Per-column speed divisor (1..3): lower = faster
  uint8_t speed[8];
  // Per-column tick counter
  uint8_t tick[8];
  for (uint8_t x = 0; x < W; x++) {
    headY[x] = 0xFF; // inactive
    speed[x] = (uint8_t)(1 + (nextL() % 3));
    tick[x] = 0;
  }

  // Initial gentle clear
  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Trail fade: higher value = faster fade; keep trails visible a bit
    matrixFade(70);

    for (uint8_t x = 0; x < W; x++) {
      // Possibly spawn a new head at top if inactive
      if (headY[x] == 0xFF) {
        // Spawn chance ~ 1/5 per frame per column
        if ((nextL() & 0x07) == 0) {
          headY[x] = 0; // top
          speed[x] = (uint8_t)(1 + (nextL() % 3));
          tick[x] = 0;
        }
      } else {
        // Advance based on speed divisor
        tick[x]++;
        if ((tick[x] % speed[x]) == 0) {
          if (headY[x] + 1 < H) headY[x]++;
          else {
            // At bottom: either continue off then deactivate, or immediately reset
            headY[x] = 0xFF; // deactivate; next loop may respawn
            continue;
          }
        }
        // Draw head and a dimmer body just above (if exists)
        uint8_t y = headY[x];
        uint32_t headCol = pixels.Color(170, 255, 170); // bright green with hint of white
        matrixSet(x, y, headCol);
        if (y > 0) {
          uint32_t bodyCol = pixels.Color(40, 180, 60);
          matrixSet(x, (uint8_t)(y - 1), dimColor(bodyCol, 200));
        }
      }
    }

    // Rare bright glyph flicker on random pixels to mimic characters
    if ((nextL() & 0x0F) == 0) {
      uint8_t rx = (uint8_t)(nextL() % W);
      uint8_t ry = (uint8_t)(nextL() % H);
      matrixSet(rx, ry, pixels.Color(120, 255, 120));
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixWaterfall
   A misty/spraying waterfall adapted to 5x4:
   - Soft blue drops fall in each column leaving trails.
   - On hitting the bottom, a light white/blue mist spreads sideways.
   - Continuous subtle seep along bottom row for a wet look.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixWaterfall(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;

  // Small LFSR for pseudo-randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0x42AF);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Per-column falling head (0..H-1) or 0xFF when inactive
  uint8_t headY[8];
  uint8_t speed[8];   // 1..3 (lower is faster)
  uint8_t tick[8];
  for (uint8_t x = 0; x < W; x++) {
    headY[x] = 0xFF; // inactive
    speed[x] = (uint8_t)(1 + (nextL() % 3));
    tick[x] = 0;
  }

  // Gentle clear
  matrixFill(0);
  pixels.show();

  // Bottom foam persistence per column (0..255 intensity)
  uint8_t foam[8];
  for (uint8_t i = 0; i < W; i++) foam[i] = 0;

  // Phase for color crossfade (blue <-> pink)
  uint8_t t = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Trails fade; a bit slower to keep more mist visible
    matrixFade(80);

    // Triangle wave 0..255
    uint8_t ph = (uint8_t)(t);
    uint8_t tri = (ph & 0x80) ? (uint8_t)(255 - ((ph & 0x7F) << 1))
                              : (uint8_t)(((ph & 0x7F) << 1));

    // Helper to blend two RGB colors by weight w (0..255)
    auto blend = [&](uint32_t a, uint32_t b, uint8_t w) -> uint32_t {
      uint8_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
      uint8_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
      uint8_t r = (uint16_t)ar * (255 - w) / 255 + (uint16_t)br * w / 255;
      uint8_t g = (uint16_t)ag * (255 - w) / 255 + (uint16_t)bg * w / 255;
      uint8_t bch = (uint16_t)ab * (255 - w) / 255 + (uint16_t)bb * w / 255;
      return pixels.Color(r, g, bch);
    };

    // Define blue and pink palettes, then blend by tri
    const uint32_t HEAD_BLUE  = pixels.Color(120, 180, 255);
    const uint32_t HEAD_PINK  = pixels.Color(255, 120, 200);
    const uint32_t BODY_BLUE  = pixels.Color(40, 100, 220);
    const uint32_t BODY_PINK  = pixels.Color(220, 60, 140);
    const uint32_t MISTA_BLUE = pixels.Color(200, 220, 255);
    const uint32_t MISTA_PINK = pixels.Color(255, 200, 230);
    const uint32_t MISTB_BLUE = pixels.Color(120, 180, 255);
    const uint32_t MISTB_PINK = pixels.Color(255, 140, 200);
    const uint32_t FOAM_BLUE  = pixels.Color(200, 230, 255);
    const uint32_t FOAM_PINK  = pixels.Color(255, 220, 240);
    const uint32_t SPARK_BLUE = pixels.Color(220, 240, 255);
    const uint32_t SPARK_PINK = pixels.Color(255, 230, 245);

    uint32_t COL_HEAD  = blend(HEAD_BLUE,  HEAD_PINK,  tri);
    uint32_t COL_BODY  = blend(BODY_BLUE,  BODY_PINK,  tri);
    uint32_t COL_MISTA = blend(MISTA_BLUE, MISTA_PINK, tri);
    uint32_t COL_MISTB = blend(MISTB_BLUE, MISTB_PINK, tri);
    uint32_t COL_FOAM  = blend(FOAM_BLUE,  FOAM_PINK,  tri);
    uint32_t COL_SPARK = blend(SPARK_BLUE, SPARK_PINK, tri);

    for (uint8_t x = 0; x < W; x++) {
      // Spawn new drop at the top occasionally when inactive
      if (headY[x] == 0xFF) {
        if ((nextL() & 0x07) <= 1) { // ~2/8 chance
          headY[x] = 0; // top
          speed[x] = (uint8_t)(1 + (nextL() % 3));
          tick[x] = 0;
        }
        continue;
      }

      // Progress falling based on per-column speed divider
      tick[x]++;
      if ((tick[x] % speed[x]) == 0) {
        if (headY[x] + 1 < H) headY[x]++;
        else {
          // Splash at bottom: brief mist to sides and above
          uint8_t y = (uint8_t)(H - 1);
          matrixSet(x, y, dimColor(COL_MISTA, 160));
          if (x > 0)      matrixSet((uint8_t)(x - 1), y, dimColor(COL_MISTB, 140));
          if (x + 1 < W)  matrixSet((uint8_t)(x + 1), y, dimColor(COL_MISTB, 140));
          if (y > 0)      matrixSet(x, (uint8_t)(y - 1), dimColor(COL_MISTB, 120));
          // Chance of extra spray pixel popping up one row higher and to the side
          if ((nextL() & 0x03) <= 1) { // slightly more spray
            int8_t sx = (int8_t)x + (int8_t)((nextL() & 1) ? 1 : -1);
            uint8_t sy = (uint8_t)(y > 0 ? y - 1 : y);
            if (sx >= 0 && sx < (int8_t)W) matrixSet((uint8_t)sx, sy, dimColor(COL_MISTA, 200));
          }
          // Add foam persistence at the bottom with lateral push
          if (foam[x] < 220) foam[x] = 220;
          if (x > 0 && foam[x-1] < 140) foam[x-1] = 140;
          if (x + 1 < W && foam[x+1] < 140) foam[x+1] = 140;
          // Deactivate head; new one may spawn later
          headY[x] = 0xFF;
          continue;
        }
      }

      // Draw falling head and faint body above
      uint8_t y = headY[x];
      matrixSet(x, y, dimColor(COL_HEAD, 220));
      if (y > 0) matrixSet(x, (uint8_t)(y - 1), dimColor(COL_BODY, 160));
    }

    // Persistent foam rendering and shimmer along the bottom row
    for (uint8_t x = 0; x < W; x++) {
      // Light diffusion: nudge foam sideways a bit
      if ((nextL() & 0x07) == 0) {
        if ((nextL() & 1) && x + 1 < W && foam[x] > 10) { uint8_t d = (uint8_t)(foam[x] / 6); foam[x] -= d; if (foam[x+1] + d < 255) foam[x+1] += d; }
        else if (x > 0 && foam[x] > 10) { uint8_t d = (uint8_t)(foam[x] / 6); foam[x] -= d; if (foam[x-1] + d < 255) foam[x-1] += d; }
      }
      // Decay foam slowly
      if (foam[x] > 4) foam[x] = (uint8_t)(foam[x] - 4); else foam[x] = 0;
      // Base foam color (white-blue) scaled by foam intensity
      if (foam[x] > 0) matrixSet(x, (uint8_t)(H - 1), dimColor(COL_FOAM, foam[x]));

      // Occasional sparkle and tiny up-splash
      uint8_t rnd = (uint8_t)(nextL() & 0xFF);
      if ((rnd & 0x07) == 0) {
        uint8_t sparkle = (uint8_t)(160 + (rnd & 0x1F));
        matrixSet(x, (uint8_t)(H - 1), dimColor(COL_SPARK, sparkle));
      } else if ((rnd & 0x1F) == 0) {
        if (H >= 2) matrixSet(x, (uint8_t)(H - 2), dimColor(pixels.Color(140, 200, 255), 140));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t += 3; // advance color phase slowly
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixNightStreet2000
   Night street scene on 5x4:
   - Warm streetlights cast a vertical glow with subtle flicker.
   - Passing car headlights sweep across the bottom row (paired white).
   - Neon sign flickers in pink/cyan tones in a small 2x2 area.
   - Distant window twinkles in the top rows.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
static inline void matrixNightStreet2000(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W, H = MATRIX_H;
  // LFSR for light randomness
  uint16_t lfsr = (uint16_t)(millis() ^ 0xA55A);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Car headlights state: two cars with x position and direction
  int8_t carX0 = -3, carX1 = W + 2; // start off-screen
  int8_t carDir0 = 1;               // left->right
  int8_t carDir1 = -1;              // right->left
  uint8_t tick = 0;

  // Neon sign area (2x2) anchored near top-right if possible
  const uint8_t neonX = (W >= 4) ? (W - 2) : 0;
  const uint8_t neonY = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Fade trails gently
    matrixFade(72);

    // Base night tint (very dark blue), blended sparsely to save cycles
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        if (((x + y + tick) & 0x0F) == 0) {
          uint32_t tint = pixels.Color(5, 8, 20);
          int idx = matrixIndex(x, y);
          uint32_t prev = pixels.getPixelColor(idx);
          uint8_t a = 16; // small blend-in
          uint8_t pr = (prev >> 16) & 0xFF, pg = (prev >> 8) & 0xFF, pb = prev & 0xFF;
          uint8_t tr = (tint >> 16) & 0xFF, tg = (tint >> 8) & 0xFF, tb = tint & 0xFF;
          uint8_t r = (uint16_t)pr * (255 - a) / 255 + (uint16_t)tr * a / 255;
          uint8_t g = (uint16_t)pg * (255 - a) / 255 + (uint16_t)tg * a / 255;
          uint8_t b = (uint16_t)pb * (255 - a) / 255 + (uint16_t)tb * a / 255;
          pixels.setPixelColor(idx, r, g, b);
        }
      }
    }

    // Streetlights: choose a column and cast warm vertical glow with flicker
    if ((tick & 0x03) == 0) {
      uint8_t col = (uint8_t)(nextL() % W);
      uint8_t base = (uint8_t)(180 + (nextL() & 0x1F)); // 180..211
      for (uint8_t y = 0; y < H; y++) {
        uint8_t lvl = (uint8_t)(base - y * 24); // stronger at bottom
        if (lvl < 40) lvl = 40;
        matrixSet(col, y, dimColor(pixels.Color(255, 200, 80), lvl));
      }
    }

    // Car headlights sweeping on bottom row (two cars, opposite directions)
    if ((tick & 0x01) == 0) {
      carX0 += carDir0; carX1 += carDir1;
      if (carX0 >= (int8_t)W + 2) carX0 = -3;   // wrap
      if (carX1 < -3)            carX1 = (int8_t)W + 2; // wrap
    }
    auto drawCar = [&](int8_t cx, uint32_t headCol) {
      for (int8_t k = 0; k < 2; k++) {
        int8_t x = (int8_t)(cx + k);
        if (x >= 0 && x < (int8_t)W) matrixSet((uint8_t)x, (uint8_t)(H - 1), headCol);
      }
    };
    uint32_t headCol0 = pixels.Color(255, 255, 220);
    uint32_t headCol1 = pixels.Color(220, 240, 255);
    drawCar(carX0, dimColor(headCol0, 220));
    drawCar(carX1, dimColor(headCol1, 200));

    // Neon sign flicker in pink/cyan (2x2 area)
    uint8_t neonPhase = (uint8_t)(nextL() & 0xFF);
    uint8_t neonOn = (uint8_t)((neonPhase & 0x1F) > 3); // occasionally off
    if (neonOn) {
      uint32_t neonA = pixels.Color(255, 80, 160); // pink
      uint32_t neonB = pixels.Color(40, 200, 200); // cyan
      uint32_t neonMix = (neonPhase & 0x20) ? neonA : neonB;
      uint8_t pul = (uint8_t)(180 + (neonPhase & 0x3F));
      for (uint8_t dy = 0; dy < 2 && neonY + dy < H; dy++)
        for (uint8_t dx = 0; dx < 2 && neonX + dx < W; dx++)
          matrixSet((uint8_t)(neonX + dx), (uint8_t)(neonY + dy), dimColor(neonMix, pul));
    }

    // Distant windows twinkle in top half
    if ((tick & 0x03) == 0) {
      uint8_t wx = (uint8_t)(nextL() % W);
      uint8_t wy = (uint8_t)(nextL() % (H > 2 ? (H - 2) : H)); // avoid bottom row
      matrixSet(wx, wy, dimColor(pixels.Color(255, 220, 120), (uint8_t)(140 + (nextL() & 0x3F))));
    }

    pixels.show();
    delay(stepDelay_ms);
    tick++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* matrixSunsetPickleSun
   Sunset gradient sky with a small "cucumber" sun composed of green rings.
   - Warm sky fades from pink/magenta (top) to orange/yellow (horizon) over time.
   - Sun is a 2x2 disc centered near the horizon with cucumber-green tones and subtle sparkle.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixSunsetPickleSun(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint8_t t = 0;

  // Helper to blend two RGB colors (8-bit channels) by weight w (0..255)
  auto blend = [&](uint32_t a, uint32_t b, uint8_t w) -> uint32_t {
    uint8_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
    uint8_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
    uint8_t r = (uint16_t)ar * (255 - w) / 255 + (uint16_t)br * w / 255;
    uint8_t g = (uint16_t)ag * (255 - w) / 255 + (uint16_t)bg * w / 255;
    uint8_t bch = (uint16_t)ab * (255 - w) / 255 + (uint16_t)bb * w / 255;
    return pixels.Color(r, g, bch);
  };

  // Sky palette endpoints
  const uint32_t SKY_TOP_A = pixels.Color(255, 60, 160);  // pink/magenta
  const uint32_t SKY_TOP_B = pixels.Color(255, 120, 60);  // warm orange
  const uint32_t SKY_HOR_A = pixels.Color(255, 180, 40);  // yellow-orange
  const uint32_t SKY_HOR_B = pixels.Color(255, 120, 0);   // burnt orange

  // Cucumber sun greens (inner brighter, outer darker)
  const uint32_t SUN_INNER = pixels.Color(120, 230, 120);
  const uint32_t SUN_RING  = pixels.Color(40, 160, 80);

  // Sun horizontal center stays centered; vertical center will be animated
  const uint8_t cx = MATRIX_W / 2;   // 2 on 5-wide

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Animate sky palette over time via a slow triangle
    uint8_t ph = (uint8_t)(t * 4);           // slow phase
    // inline triangle wave 0..255 without external helper
    uint8_t tri = (ph & 0x80) ? (uint8_t)(255 - ((ph & 0x7F) << 1))
                              : (uint8_t)(((ph & 0x7F) << 1));

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      // row weight: 0 (top) -> 255 (bottom)
      uint8_t roww = (uint8_t)((uint16_t)y * 255 / (MATRIX_H - 1));
      // Interpolate top and horizon palettes separately, then mix by row
      uint32_t topCol = blend(SKY_TOP_A, SKY_TOP_B, tri);
      uint32_t horCol = blend(SKY_HOR_A, SKY_HOR_B, (uint8_t)(255 - tri));
      uint32_t skyCol = blend(topCol, horCol, roww);
      // Top should be a bit darker, bottom full; scale ~200..255 over rows
      uint8_t rowDim = (uint8_t)(200 + ((uint16_t)y * 55 / (MATRIX_H - 1))); // top ~200, bottom 255
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        matrixSet(x, y, dimColor(skyCol, rowDim));
      }
    }

    // Compute animated sun vertical center as a descending sawtooth (sunset), then restart at top
    static uint8_t sunPhase = 0;               // 0..255
    const uint8_t horizonRow = (uint8_t)(MATRIX_H - 1); // treat bottom row as horizon line
    // Map phase to 0..horizonRow+1 (allows one extra step "below" horizon before restart)
    uint8_t cy = (uint8_t)(((uint16_t)sunPhase * (horizonRow + 1)) / 255);
    bool fullyBelow = (cy > horizonRow); // strictly below -> hidden
    // Dwell counters to keep the sun hidden longer: first fade darker, then hold night
    static uint8_t dwell = 0;   // counts total dwell frames after sunset

    // Draw the cucumber sun (2x2 disc) with green ring accent, clipped strictly above horizon
    if (!fullyBelow) {
      for (int8_t dy = -1; dy <= 0; dy++) {
        for (int8_t dx = -1; dx <= 0; dx++) {
          uint8_t x = (uint8_t)(cx + dx);
          uint8_t y = (uint8_t)(cy + dy);
          if (x < MATRIX_W && y < MATRIX_H && y < horizonRow) {
            uint8_t md = (uint8_t)(abs(dx) + abs(dy));
            uint32_t col = (md == 0) ? SUN_INNER : SUN_RING;
            uint8_t pulse = (uint8_t)(200 + ((t * 9 + (dx+1)*7 + (dy+1)*5) & 0x2F));
            // As we approach the horizon (y close to horizonRow-1), dim slightly
            uint8_t edgeDim = (y >= (uint8_t)(horizonRow - 1)) ? (uint8_t)(pulse * 4 / 5) : pulse;
            matrixSet(x, y, dimColor(col, edgeDim));
          }
        }
      }
    }

    // Tiny sparkle seeds around the sun (only when visible)
    if (!fullyBelow && ((t & 0x03) == 0x00)) {
      uint8_t sx = (uint8_t)(cx + ((t >> 2) & 0x01 ? 1 : -1));
      uint8_t sy = (uint8_t)(cy + (((t >> 3) & 0x01) ? 1 : 0));
      if (sx < MATRIX_W && sy < MATRIX_H && sy < horizonRow) matrixSet(sx, sy, pixels.Color(255, 255, 200));
    }

    // During hidden dwell, darken the entire sky gradually, then hold much darker night
    if (fullyBelow) {
      // Phase 1: fade darker over ~40 frames; Phase 2: hold for ~80 frames
      uint8_t dusk;
      if (dwell < 40) {
        dusk = (uint8_t)(220 - (dwell * 3)); // 220 -> ~100
      } else {
        dusk = 96; // hold darkest
      }
      // Night tint: gradually blend the whole sky toward deep blue while dimming
      const uint32_t NIGHT_BLUE = pixels.Color(10, 20, 60);
      // nightW grows with dwell to pull colors toward NIGHT_BLUE
      uint8_t nightW = (uint8_t)(dwell < 40 ? (dwell * 5) : 200); // 0..~200
      for (uint8_t y = 0; y < MATRIX_H; y++) {
        for (uint8_t x = 0; x < MATRIX_W; x++) {
          int idx = matrixIndex(x, y);
          uint32_t c = pixels.getPixelColor(idx);
          uint32_t cTint = blend(c, NIGHT_BLUE, nightW);
          pixels.setPixelColor(idx, dimColor(cTint, dusk));
        }
      }

      // Add more blinking stars across the entire sky region (above horizon), slower and softer
      static uint16_t starLfsr = 0xACE1; // simple LFSR for pseudo-random stars
      static uint8_t starTick = 0;       // slower tick for twinkle
      uint8_t syMax = horizonRow; // stars only above horizon
      // Update stars only every 3rd dwell frame to reduce hectic flicker
      if ((dwell % 3) == 0 && syMax > 0) {
        // advance LFSR sparsely
        starLfsr ^= (uint16_t)((t << 7) ^ (dwell * 13));
        for (uint8_t k = 0; k < 2; k++) { // fewer stars per update
          // LFSR step (x^16 + x^14 + x^13 + x^11)
          uint16_t bit = (uint16_t)(((starLfsr >> 0) ^ (starLfsr >> 2) ^ (starLfsr >> 3) ^ (starLfsr >> 5)) & 1);
          starLfsr = (uint16_t)((starLfsr >> 1) | (bit << 15));
          uint8_t sx = (uint8_t)(starLfsr % MATRIX_W);
          uint8_t sy = (uint8_t)((starLfsr >> 8) % syMax); // 0..horizonRow-1
          // Slow twinkle based on starTick triangle 120..200
          uint8_t ph = (uint8_t)(starTick & 0xFF);
          uint8_t tri = (ph & 0x80) ? (uint8_t)(200 - (((ph & 0x7F) << 1) >> 1))
                                    : (uint8_t)(120 + (((ph & 0x7F) << 1) >> 1));
          int sidx = matrixIndex(sx, sy);
          uint32_t prev = pixels.getPixelColor(sidx);
          uint32_t target = dimColor(pixels.Color(255, 255, 220), tri);
          // Blend in softly so stars fade in/out rather than snap
          uint8_t alpha = 64; // ~25% towards target per update
          uint32_t blended = blend(prev, target, alpha);
          pixels.setPixelColor(sidx, blended);
        }
        starTick += 5; // slow increment
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    // advance sun phase; when hidden below horizon, dwell longer, then restart at the top
    if (fullyBelow) {
      if (dwell < 120) { dwell++; }  // ~40 fade + ~80 hold
      else { dwell = 0; sunPhase = 0; }
    } else {
      sunPhase = (uint8_t)(sunPhase + 3);
    }
    t++;
  }
}
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
/* matrixCoteAzur
   French coastal landscape on 5x4: deep blue sea with shimmering waves at bottom,
   green coastline band above, peppered with rock-gray accents. Gentle motion.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixCoteAzur(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint32_t SEA_BASE   = pixels.Color(20, 60, 160);   // deep blue
  const uint32_t SEA_LIGHT  = pixels.Color(60, 120, 220);  // light blue shimmer
  const uint32_t COAST_GRN  = pixels.Color(30, 180, 90);   // lush green

  uint32_t start = millis();
  uint8_t t = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(60);
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        if (y <= 1) {
          uint8_t phase = (uint8_t)(x * 40 + t * 10);
          uint8_t w = tri8_local(phase);           // 0..255
          uint8_t rB = (SEA_BASE >> 16) & 0xFF, gB = (SEA_BASE >> 8) & 0xFF, bB = SEA_BASE & 0xFF;
          uint8_t rL = (SEA_LIGHT >> 16) & 0xFF, gL = (SEA_LIGHT >> 8) & 0xFF, bL = SEA_LIGHT & 0xFF;
          uint8_t r = (uint16_t)rB * (255 - w) / 255 + (uint16_t)rL * w / 255;
          uint8_t g = (uint16_t)gB * (255 - w) / 255 + (uint16_t)gL * w / 255;
          uint8_t b = (uint16_t)bB * (255 - w) / 255 + (uint16_t)bL * w / 255;
          matrixSet(x, y, pixels.Color(r, g, b));
        } else if (y == 2) {
          uint8_t rnd = (uint8_t)((x * 61 + t * 17) & 0xFF);
          bool rock = (rnd & 0x1F) == 0; // ~1/32 chance per frame
          if (rock) {
            uint8_t k = (uint8_t)(80 + (rnd & 0x3F));
            matrixSet(x, y, pixels.Color(k, k, k));
          } else {
            uint8_t pulse = (uint8_t)(180 + ((t + x * 9) & 0x2F));
            matrixSet(x, y, dimColor(COAST_GRN, pulse));
          }
        } else { // y == 3
          uint32_t hint = pixels.Color(10, 40, 60);
          uint8_t pulse = (uint8_t)(120 + ((t + x * 7) & 0x1F));
          matrixSet(x, y, dimColor(hint, pulse));
        }
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
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
/* matrixExplosion
   A nasty explosion: blinding flash, expanding shockwave, flying debris, smoky afterglow.
   Params: runtime_ms, stepDelay_ms
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
inline void matrixExplosion(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;

  // Small LFSR for randomness (center + debris)
  uint16_t lfsr = (uint16_t)(millis() ^ 0xBEEF);
  auto nextL = [&]() {
    uint16_t b = (uint16_t)(((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1);
    lfsr = (uint16_t)((lfsr >> 1) | (b << 15));
    return lfsr;
  };

  // Randomize explosion center, avoid direct edges
  uint8_t cx;
  uint8_t cy;
  if (W > 2) cx = (uint8_t)(1 + (nextL() % (W - 2))); else cx = (uint8_t)(nextL() % W);
  if (H > 2) cy = (uint8_t)(1 + (nextL() % (H - 2))); else cy = (uint8_t)(nextL() % H);

  // Debris particles (very few to save RAM)
  const uint8_t N = 6;
  int16_t px[N], py[N];   // position in 1/16 pixels
  int8_t  vx[N], vy[N];   // velocity in 1/16 pixels per frame
  uint8_t life[N];
  for (uint8_t i = 0; i < N; i++) {
    px[i] = (int16_t)(cx * 16);
    py[i] = (int16_t)(cy * 16);
    int8_t ang = (int8_t)(nextL() & 0x1F);     // 0..31
    int8_t spd = (int8_t)(16 + (int8_t)(nextL() & 0x0F)); // 16..31
    // Approx distribute vx,vy on 8 directions without trig
    int8_t dir = (int8_t)(ang & 7);
    const int8_t v8x[8] = { 16, 12,  0, -12, -16, -12,  0,  12 };
    const int8_t v8y[8] = {  0, 12, 16,  12,   0, -12,-16, -12 };
    vx[i] = (int8_t)((int16_t)v8x[dir] * spd / 24);
    vy[i] = (int8_t)((int16_t)v8y[dir] * spd / 24);
    life[i] = (uint8_t)(40 + (nextL() & 0x1F));
  }

  // Phase 0: blinding flash
  matrixFill(pixels.Color(255,255,255));
  pixels.show();
  delay((uint16_t)(stepDelay_ms / 2));
  matrixFade(140);
  pixels.show();
  delay((uint16_t)(stepDelay_ms / 2));

  uint8_t radius = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Gentle decay to create smoke and afterglow
    matrixFade(220);

    // Expanding shockwave ring
    uint8_t r2 = (uint8_t)(radius * radius);
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int8_t dx = (int8_t)x - (int8_t)cx;
        int8_t dy = (int8_t)y - (int8_t)cy;
        // squared distance without lambda to avoid unused warning
        uint16_t d2 = (uint16_t)((int16_t)dx * (int16_t)dx + (int16_t)dy * (int16_t)dy);
        // draw ring for |d^2 - r^2| within band
        int16_t diff = (int16_t)d2 - (int16_t)r2;
        if (diff < 0) diff = (int16_t)-diff;
        if (diff <= 2) {
          // hot color near core to cooler orange as it expands
          uint8_t heat = (uint8_t)(255 - (radius * 20)); // decreases with radius
          if (heat < 40) heat = 40;
          // heat to color: white/yellow -> orange/red
          uint8_t r = 255;
          uint8_t g = (uint8_t)(heat);
          uint8_t b = (uint8_t)(heat / 6);
          matrixSet(x, y, pixels.Color(r, g, b));
        }
      }
    }

    // Debris update and draw
    for (uint8_t i = 0; i < N; i++) {
      if (!life[i]) continue;
      // Integrate
      px[i] += vx[i];
      py[i] += vy[i];
      // gravity pull down
      if ((millis() & 1) == 0) vy[i] += 1; // tiny gravity
      // air drag
      vx[i] = (int8_t)((int16_t)vx[i] * 15 / 16);
      vy[i] = (int8_t)((int16_t)vy[i] * 15 / 16);
      // convert to pixel
      int8_t ix = (int8_t)(px[i] / 16);
      int8_t iy = (int8_t)(py[i] / 16);
      if (ix >= 0 && ix < (int8_t)W && iy >= 0 && iy < (int8_t)H) {
        uint8_t lv = life[i];
        uint8_t rr = (uint8_t)(200 + (lv));
        if (rr > 255) rr = 255;
        uint8_t gg = (uint8_t)(80 + (lv / 2));
        uint8_t bb = (uint8_t)(20 + (lv / 8));
        matrixSet((uint8_t)ix, (uint8_t)iy, pixels.Color(rr, gg, bb));
      }
      if (life[i] > 0) life[i]--;
    }

    // Slight smoky blue-gray drift overlay
    if ((radius & 1) == 0) {
      for (uint8_t x = 0; x < W; x++) {
        uint8_t y = (uint8_t)((x + radius) % H);
        uint32_t c = pixels.Color(30, 40, 60);
        matrixSet(x, y, dimColor(c, 80));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    radius++;
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

// Dual-row Larson scanner: top goes L->R, bottom goes R->L, both with tails
inline void matrixLarsonScannerDual(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t yTop = 0;
  const uint8_t yBot = (uint8_t)(MATRIX_H - 1);
  int8_t posTop = 0, dirTop = 1;
  int8_t posBot = (int8_t)(MATRIX_W - 1), dirBot = -1;
  uint8_t hue = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(56);
    uint32_t headCol = Wheel(hue);
    uint32_t tail1 = dimColor(headCol, 160);
    uint32_t tail2 = dimColor(headCol, 80);
    // Top row
    matrixSet((uint8_t)posTop, yTop, headCol);
    if (posTop - 1 >= 0) matrixSet((uint8_t)(posTop - 1), yTop, tail1);
    if (posTop - 2 >= 0) matrixSet((uint8_t)(posTop - 2), yTop, tail2);
    // Bottom row
    matrixSet((uint8_t)posBot, yBot, headCol);
    if (posBot + 1 < (int8_t)MATRIX_W) matrixSet((uint8_t)(posBot + 1), yBot, tail1);
    if (posBot + 2 < (int8_t)MATRIX_W) matrixSet((uint8_t)(posBot + 2), yBot, tail2);

    pixels.show();
    delay(stepDelay_ms);

    // Advance
    posTop += dirTop; posBot += dirBot; hue += 5;
    // Bounce
    if (posTop <= 0) { posTop = 0; dirTop = 1; }
    if (posTop >= (int8_t)(MATRIX_W - 1)) { posTop = (int8_t)(MATRIX_W - 1); dirTop = -1; }
    if (posBot <= 0) { posBot = 0; dirBot = 1; }
    if (posBot >= (int8_t)(MATRIX_W - 1)) { posBot = (int8_t)(MATRIX_W - 1); dirBot = -1; }
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
    // Allow y < 0 (above top) to be empty so pieces can spawn/fall into view
    if (x < 0 || x >= (int8_t)W || y >= (int8_t)H) return true; // walls and floor are solid
    if (y < 0) return false; // above top is empty space
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
    // Safe spawn ranges per piece to avoid immediate OOB when writing
    int8_t px;
    if (kind == 0) { // 2x2 square spans x..x+1
      px = (int8_t)random(0, (int8_t)(W >= 2 ? (W - 1) : 0));
    } else if (kind == 1) { // 3-long bar spans x..x+2
      px = (int8_t)random(0, (int8_t)(W >= 3 ? (W - 2) : 0));
    } else { // L-ish spans x..x+2 on bottom row
      px = (int8_t)random(0, (int8_t)(W >= 3 ? (W - 2) : 0));
    }
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
        // Lock piece into field; guard bounds to avoid OOB writes
        if (kind == 0) {
          for (uint8_t i = 0; i < 4; i++) {
            int8_t ax = (int8_t)(px + P0[i][0]);
            int8_t ay = (int8_t)(py + P0[i][1]);
            if (ax >= 0 && ax < (int8_t)W && ay >= 0 && ay < (int8_t)H) field[idx(ax, ay)] = 1;
          }
        } else if (kind == 1) {
          for (uint8_t i = 0; i < 3; i++) {
            int8_t ax = (int8_t)(px + P1[i][0]);
            int8_t ay = (int8_t)(py + P1[i][1]);
            if (ax >= 0 && ax < (int8_t)W && ay >= 0 && ay < (int8_t)H) field[idx(ax, ay)] = 1;
          }
        } else {
          for (uint8_t i = 0; i < 4; i++) {
            int8_t ax = (int8_t)(px + P2[i][0]);
            int8_t ay = (int8_t)(py + P2[i][1]);
            if (ax >= 0 && ax < (int8_t)W && ay >= 0 && ay < (int8_t)H) field[idx(ax, ay)] = 1;
          }
        }
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
      if (kind == 0) {
        for (uint8_t i = 0; i < 4; i++) { int8_t ax = px + P0[i][0], ay = py + P0[i][1]; if (ax >= 0 && ax < (int8_t)W && ay >= 0 && ay < (int8_t)H) matrixSet((uint8_t)ax, (uint8_t)ay, col); }
      } else if (kind == 1) {
        for (uint8_t i = 0; i < 3; i++) { int8_t ax = px + P1[i][0], ay = py + P1[i][1]; if (ax >= 0 && ax < (int8_t)W && ay >= 0 && ay < (int8_t)H) matrixSet((uint8_t)ax, (uint8_t)ay, col); }
      } else {
        for (uint8_t i = 0; i < 4; i++) { int8_t ax = px + P2[i][0], ay = py + P2[i][1]; if (ax >= 0 && ax < (int8_t)W && ay >= 0 && ay < (int8_t)H) matrixSet((uint8_t)ax, (uint8_t)ay, col); }
      }
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
