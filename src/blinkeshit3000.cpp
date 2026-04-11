#include "matrix_helpers.h"
#include "visuals.h"
#include <Adafruit_NeoPixel.h>
#include <avr/pgmspace.h>
#include <neolib.h>

// Compile-time selection of a single visual for batch WAV generation.
// Define VISUAL_ID via build flags to pick which visual runs.
// When not defined, the default selection below in loop() remains active.
#ifndef VISUAL_ID
#define VISUAL_ID -1
#endif

#ifndef GLOBAL_BRIGHTNESS
#define GLOBAL_BRIGHTNESS 255
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Button-driven effect switching (using neolib's buttons)
========================================================================================================================
   _________            _                 _
  |  mode  |   utils  | |__   __ _  __ _| |_ ___  _ __
  |  o___o |          | '_ \ / _` |/ _` | __/ _ \| '__|
  |__/___\_|          | | | | (_| | (_| | || (_) | |
                      |_| |_|\__,_|\__,_|\__\___/|_|
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static uint8_t g_mode = 0;
static const uint8_t EFFECT_COUNT = 7; // adjust if you add/remove effects

void changeMode(int8_t delta) {
  int16_t m = (int16_t)g_mode + delta;
  if (m < 0)
    m = EFFECT_COUNT - 1;
  if (m >= EFFECT_COUNT)
    m = 0;
  g_mode = (uint8_t)m;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* the setup routine runs once when you start the tape or press reset
========================================================================================================================
   _________            _
  | setup() |   ___ ___| |_ _ _ _ _
  |  o___o  |  |_ -| -_|  _| | | . |
  |__/___\__|  |___|___|_| |___|  _|
                               |_|
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setup(void) {

  pinMode(SPEAKERPIN, OUTPUT);
  uint8_t brightness = GLOBAL_BRIGHTNESS;
  neobegin();
  pixels.setBrightness(brightness);
  matrixFill(0);
  pixels.show();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* The main loop to put all your code
========================================================================================================================
   _________    _
  | loop()  |  | |___ ___ ___
  |  o___o  |  | | . | . | . |
  |__/___\__|  |_|___|___|  _|
                         |_|
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void loop(void) {
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /* Visual Selector (uncomment ONE)
  ========================================================================================================================
   ___    _ _____ _             _              _ _
  / _ \  / |_   _(_)_ __   __ _| |_ _   _  ___| (_)_ __
 | | | | | | | | | | '_ \ / _` | __| | | |/ __| | | '_ \
 | |_| | | | | | | | | | | (_| | |_| |_| | (__| | | | | |
  \___/  |_| |_| |_|_| |_|\__,_|\__|\__,_|\___|_|_|_| |_|

                 AI-tiny  8bit  slop

    Each call uses (runtime_ms, speed/stepDelay params ...)

    Lightweight visuals:
    //matrixRainbowZoom(12000, 80);     // concentric rainbow rings zoom
    //matrixPinkSpiral(12000, 60);      // pink dot traces inward spiral
    //matrixBouncingDot(10000, 80);     // single dot bounces with trail
    //matrixRain(10000, 60);            // blue drops fall with fading tails
    //matrixTwinkle(10000, 60);         // soft random multicolor twinkles
    //matrixLarsonScanner(10000, 120);  // Cylon scanner with tails
    //matrixPong(12000, 60);            // mini Pong with AI paddles

    Themed/new visuals:
    //matrixKanjiScroll(12000, 90);     // kanji-like glyph scroll
    //matrixExplosion(6000, 80);        // nasty explosion: flash, shockwave,
 debris, smoke
    //matrixDigitalRain(12000, 90);     // Matrix-style green code rain for 5x4
    //matrixShoggoth(12000, 90);        // wobbling blob with blinking eyes
    //matrixDnBDancer(12000, 290);      // two-frame dancer with bass pulse

    Flag engine:
    //matrixFlagsShowFade(14000, 900, 350, 6);  // cycle flags with cross-fade
    //matrixFlagsShow(12000, 800);              // cycle flags without fade

    Heavier visuals (enable only if flash allows):
    //matrixSpinner(10000, 60);        // dot runs around the perimeter
    //matrixWipe(10000, 20);           // color wipe back and forth
    //matrixCometSweep(10000, 50);     // bright comet head sweeps all pixels
    //matrixFire(12000, 160, 80, 80);  // fire with hotter center and breathing
 edges
    //matrixLightning(10000);          // random lightning strikes with glow
    //matrixGalagaInvader(10000, 120); // classic alien flapping across
    //matrixStickManWall(10000, 120);  // stick figure runs and bonks (if
 included)

    Extras:
    //matrixRainbowWaves(12000, 80);   // flowing rainbow stripes across matrix
    //matrixTetris(12000, 140);        // simple falling blocks, clear full rows
    //matrixFiveEightSeam(12000, 240, 4); // 5/8 rhythm on seam (col 4), ~250ms
 per 8th
    //matrixCoteAzur(12000, 80);       // French coastal landscape: blue sea,
 green coast, rock-gray
    //matrixSunsetPickleSun(12000, 90); // Sunset sky with cucumber-green sun
  */
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // If VISUAL_ID is provided, run the selected visual and return.
  // Otherwise, use the default preview selection below.
#if (VISUAL_ID >= 0)
  // Keep runtimes consistent so each WAV has similar length.
  // Note: Many visuals accept (runtime_ms, stepDelay_ms) or other params.
  switch (VISUAL_ID) {
  // Lightweight visuals (IDs 0..9)
  case 0:
    matrixLarsonScanner(10000, 120);
    break;
  case 1:
    matrixBouncingDot(10000, 80);
    break;
  case 2:
    matrixRain(10000, 60);
    break;
  case 3:
    matrixTwinkle(10000, 60);
    break;
  case 4:
    matrixWipe(10000, 20);
    break;
  case 5:
    matrixSpinner(10000, 60);
    break;
  case 6:
    matrixFire(10000, 160, 80, 80);
    break;
  case 7:
    matrixRainbowZoom(10000, 80);
    break;
  case 8:
    matrixPinkSpiral(10000, 60);
    break;
  case 9:
    matrixPong(10000, 60);
    break;

  // Themed visuals (IDs 10..19)
  case 10:
    matrixDnBDancer(10000, 290);
    break;
  case 11:
    matrixKanjiScroll(10000, 90);
    break;
  case 12:
    matrixExplosion(10000, 80);
    break;
  case 13:
    matrixShoggoth(10000, 90);
    break;
  case 14:
    matrixRainbowWaves(10000, 80);
    break;
  case 15:
    matrixTetris(10000, 140);
    break;
  case 16:
    matrixFiveEightSeam(10000, 240, 4);
    break;
  case 17:
    matrixCoteAzur(10000, 80);
    break;
  case 18:
    matrixSunsetPickleSun(10000, 90);
    break;
  case 19:
    matrixLightning(10000);
    break;
  case 20:
    matrixFlagsShow(10000, 900);
    break;
  case 21:
    matrixFlagsShowFade(10000, 900, 300, 6);
    break;
  case 22:
    matrixDigitalRain(10000, 90);
    break;
  case 23:
    matrixWaterfall(10000, 90);
    break;
  case 24:
    matrixLarsonScannerDual(10000, 90);
    break;
  case 25:
    matrixNightStreet2000(10000, 80);
    break;
  case 26:
    matrixSwitchToggle(10000, 120);
    break;
  case 27:
    matrixBlueScreen(10000, 50);
    break;
  case 28:
    matrixSnowWhite(10000, 90);
    break;
  case 29:
    matrixBigBadWolf(10000, 90);
    break;
  case 30:
    matrixTricksterPlasma(10000, 90);
    break;
  case 31:
    matrixPinkSlimeSprouts(10000, 80);
    break;
  case 32:
    matrixMilkySea(10000, 60);
    break;
  case 33:
    matrixRainbowHalfDonut(10000, 80);
    break;
  case 34:
    matrixRainbowPlanet(10000, 70);
    break;
  case 35:
    matrixStarrySky(10000, 80);
    break;
  case 36:
    matrixFogNoise(10000, 90);
    break;
  case 37:
    matrixRainbowTiledNoise(10000, 80);
    break;
  case 38:
    matrixUniverseCreation(10000, 80);
    break;
  case 39:
    matrixSpear(10000, 60);
    break;
  case 40:
    matrixChristmasTreeAura(10000, 70, 0x0A);
    break;
  case 41:
    matrixCCCRocket(10000, 60);
    break;
  case 42:
    matrixBeachWave(10000, 80);
    break;
  case 43:
    matrixAntifaFlag(10000, 80);
    break;
  case 44:
    matrixTricksterPlasmaLoop(23);
    break;
  case 45:
    matrixChaoticPink(10000, 80);
    break;
  case 46:
    matrixSpringStorm(10000, 100);
    break;
  case 47:
    matrixBlueBouncingBall(10000, 40);
    break;
  case 48:
    matrixTechnoOrbit(10000, 40);
    break;
  case 49:
    matrixRocketLiftoff(10000, 60);
    break;
  case 50:
    matrixBurningSauna(10000, 80);
    break;
  case 51:
    matrixTetrisFrantic(10000, 120);
    break;
  case 52:
    matrixSchaffhauserBock(10000, 1000);
    break;
  default:
    matrixCCCRocket(10000, 60);
    break;
  }
  return; // run one visual per loop iteration to keep behavior deterministic
          // per WAV
#endif

  // Default preview (uncomment one selection above to try others)

  // matrixTricksterPlasmaLoop(23);

  // matrixBurningSauna(15000, 80);
  // matrixMagentaMiniSnake(15000, 80);  // 15 s magenta mini snake
  // matrixMagentaMiniSnake(15000, 80);
  // matrixSpringStorm(15000, 100);
  // matrixBlueBouncingBall(15000, 40);
  // matrixTechnoOrbit(15000, 40);
  // matrixRocketLiftoff(15000, 60);
  // matrixTetrisFrantic(10000, 120);
  matrixSchaffhauserBock(10000, 1000);
  // matrixCounter1to9(15000, 120);  // 15 s Zähler, 120 ms pro Frame
}
