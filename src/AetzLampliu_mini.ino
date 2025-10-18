#include <Adafruit_NeoPixel.h>
#include <neolib.h>
#include <avr/pgmspace.h>
#include "matrix_helpers.h"
#include "visuals.h"

// Compile-time selection of a single visual for batch WAV generation.
// Define VISUAL_ID via build flags to pick which visual runs.
// When not defined, the default selection below in loop() remains active.
#ifndef VISUAL_ID
#define VISUAL_ID -1
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Left-overs from measuring temperature and play family mart tune
========================================================================================================================
   _________            _                 _            
  |  mode  |   utils  | |__   __ _  __ _| |_ ___  _ __
  |  o___o |          | '_ \ / _` |/ _` | __/ _ \| '__|
  |__/___\_|          | | | | (_| | (_| | || (_) | |   
                      |_| |_|\__,_|\__,_|\__\___/|_|   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

#include <OneWire.h>
#include "pitches.h"
#define ONEWIREPIN   PB4

#define hell          255 // Brightness
#define lowTemp       10
#define maxTemp       55
#define warnTemp      15
#define warnAlarm     12 // How often it beeps when below warnTemp

  int updateSpeed = 1000; // maybe 750ms is enough, maybe not
  uint8_t warnCount = 0;
  uint8_t statusLED = (maxTemp-10) * 3;
  uint8_t tempColor = 0;
  uint8_t showPixel = 0;

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  int16_t raw;

OneWire  ds(ONEWIREPIN);  // on pin 10 (a 4.7K resistor is necessary)

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
  if (m < 0) m = EFFECT_COUNT - 1;
  if (m >= EFFECT_COUNT) m = 0;
  g_mode = (uint8_t)m;
}

void runCurrentEffect() {
  // Keep runtimes short so button presses are handled quickly
  switch (g_mode) {
    case 0: matrixLarsonScanner(600, 120); break;
    case 1: matrixBouncingDot(600, 50);   break;
    case 2: matrixRain(600, 50);          break;
    case 3: matrixTwinkle(600, 40);       break;
    case 4: matrixWipe(600, 10);          break;
    case 5: matrixSpinner(600, 30);       break;
    case 6: matrixFire(900, 60, 80, 35);  break; // new effect
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Left-overs from measuring temperature and play family mart tune
========================================================================================================================
   _________            _                 _            
  |  mode  |   utils  | |__   __ _  __ _| |_ ___  _ __
  |  o___o |          | '_ \ / _` |/ _` | __/ _ \| '__|
  |__/___\_|          | | | | (_| | (_| | || (_) | |   
                      |_| |_|\__,_|\__,_|\__\___/|_|   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// fast pin access
#define AUDIOPIN (1<<SPEAKERPIN)
#define PINLOW (PORTB&=~AUDIOPIN)
#define PINHIGH (PORTB|=AUDIOPIN)

int c[] = {
  NOTE_B4, NOTE_G4, NOTE_D4, NOTE_G4, NOTE_A4, NOTE_D5, PAUSE, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_D4, NOTE_G4 
};

int pauseBetweenNotes = 80;
int noteDuration_ms = 150;

void playMart(int notes[])
{

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote <= 11; thisNote++) {

    displayBinaryValue(notes[thisNote],pixels.Color(50,0,50));
    pixels.show(); // This sends the updated pixel color to the hardware.
    playSound( notes[thisNote], noteDuration_ms);
    setColorAllPixel(0); // pixels off (neolib)
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    delay(pauseBetweenNotes);
        
  }
  
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
  uint8_t brightness = hell;
  neobegin();
  pixels.setBrightness(brightness);
  // Quick splash screen on boot
  //matrixDemoSmiley();
  //delay(6000);
  matrixFill(0);
  pixels.show();
  
  //rainbowCycle(3,20,20);
  
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
    //matrixExplosion(6000, 80);        // nasty explosion: flash, shockwave, debris, smoke
    //matrixShoggoth(12000, 90);        // wobbling blob with blinking eyes
    //matrixDnBDancer(12000, 290);      // two-frame dancer with bass pulse

    Flag engine:
    //matrixFlagsShowFade(14000, 900, 350, 6);  // cycle flags with cross-fade
    //matrixFlagsShow(12000, 800);              // cycle flags without fade

    Heavier visuals (enable only if flash allows):
    //matrixSpinner(10000, 60);        // dot runs around the perimeter
    //matrixWipe(10000, 20);           // color wipe back and forth
    //matrixCometSweep(10000, 50);     // bright comet head sweeps all pixels
    //matrixFire(12000, 160, 80, 80);  // fire with hotter center and breathing edges
    //matrixLightning(10000);          // random lightning strikes with glow
    //matrixGalagaInvader(10000, 120); // classic alien flapping across
    //matrixStickManWall(10000, 120);  // stick figure runs and bonks (if included)

    Extras:
    //matrixRainbowWaves(12000, 80);   // flowing rainbow stripes across matrix
    //matrixTetris(12000, 140);        // simple falling blocks, clear full rows
    //matrixFiveEightSeam(12000, 240, 4); // 5/8 rhythm on seam (col 4), ~250ms per 8th
    //matrixCoteAzur(12000, 80);       // French coastal landscape: blue sea, green coast, rock-gray
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
    case 0: matrixLarsonScanner(10000, 120); break;
    case 1: matrixBouncingDot(10000, 80);   break;
    case 2: matrixRain(10000, 60);          break;
    case 3: matrixTwinkle(10000, 60);       break;
    case 4: matrixWipe(10000, 20);          break;
    case 5: matrixSpinner(10000, 60);       break;
    case 6: matrixFire(10000, 160, 80, 80); break;
    case 7: matrixRainbowZoom(10000, 80);   break;
    case 8: matrixPinkSpiral(10000, 60);    break;
    case 9: matrixPong(10000, 60);          break;

    // Themed visuals (IDs 10..19)
    case 10: matrixDnBDancer(10000, 290);      break;
    case 11: matrixKanjiScroll(10000, 90);     break;
    case 12: matrixExplosion(10000, 80);       break;
    case 13: matrixShoggoth(10000, 90);        break;
    case 14: matrixRainbowWaves(10000, 80);    break;
    case 15: matrixTetris(10000, 140);         break;
    case 16: matrixFiveEightSeam(10000, 240, 4); break;
    case 17: matrixCoteAzur(10000, 80);        break;
    case 18: matrixSunsetPickleSun(10000, 90); break;
    case 19: matrixLightning(10000);           break;

    // Flags engine (IDs 20..21)
    case 20: matrixFlagsShow(10000, 800);              break;
    case 21: matrixFlagsShowFade(10000, 900, 350, 6);  break;

    // Fallback
    default: matrixSunsetPickleSun(10000, 90); break;
  }
  return; // run one visual per loop iteration to keep behavior deterministic per WAV
#endif

  // Default preview (uncomment one selection above to try others)
  //matrixCoteAzur(8000, 80);
  matrixSunsetPickleSun(8000, 90);
  //matrixExplosion(20000, 80);
}
