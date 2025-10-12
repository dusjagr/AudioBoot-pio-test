#include <OneWire.h>
#include <Adafruit_NeoPixel.h>
#include "pitches.h"

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library


#define ONEWIREPIN   PB4
#define NEOPIXELPIN    0
#define NUMPIXELS      20
#define SPEAKERPIN     1

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

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Specific functions of the NEO-Pixel Library
 _____ _____ _____        _         _ 
|   | |   __|     |   ___|_|_ _ ___| |
| | | |   __|  |  |  | . | |_'_| -_| |
|_|___|_____|_____|  |  _|_|_,_|___|_|
                     |_|              
========================================================================================================================
   _________    
  | NEO(x)  |   void setWhiteAllPixel(uint32_t color)                   -> Sets all the pixels to the white level
  |  o___o  |   void displayBinrayValue(uint16_t value, uint32_t color) -> displays binary number
  |__/___\__|   uint32_t Wheel(byte WheelPos)                           -> Input a value 0 to 255 to get a color value.   
                                                                        The colours are a transition r - g - b - back to r. 
                                
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void setWhiteAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color, color, color);
  }
}

void setColorAllPixel(uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    pixels.setPixelColor(n, color);
  }
}

void displayBinrayValue(uint16_t value, uint32_t color)
{
  uint8_t n;
  for (n = 0; n < NUMPIXELS; n++)
  {
    if (value & (1 << n)) pixels.setPixelColor(n, color);
    //else pixels.setPixelColor(n,0); // off
  }
}

void rainbowCycle(uint8_t wait, uint8_t rounds, uint8_t rainbowPixels) {
  uint16_t i, j;

  for (j = 0; j < 256 * rounds; j++) { 
    for (i = 0; i < rainbowPixels; i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / rainbowPixels) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Create a dimmed version of a color by a scale factor (0-255)
uint32_t dimColor(uint32_t color, uint8_t scale) {
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  r = (uint16_t)r * scale / 255;
  g = (uint16_t)g * scale / 255;
  b = (uint16_t)b * scale / 255;
  return pixels.Color(r, g, b);
}

// Render a nice temperature-driven gradient across the whole strip
// and a bright white indicator at the temperature position
void renderNiceLeds(float currentCelsius) {
  // Map temperature to a hue across blue->red range
  uint8_t hue = map((int)currentCelsius, lowTemp, maxTemp, 160, 0);
  if (hue < 10) hue = 10;
  if (hue > 180) hue = 180;
  uint32_t base = Wheel(hue);

  // Background gradient: dim at start, bright at end
  for (int n = 0; n < NUMPIXELS; n++) {
    uint8_t scale = (uint16_t)(n + 1) * 255 / NUMPIXELS; // 1..255
    pixels.setPixelColor(n, dimColor(base, scale));
  }

  // White indicator ("comet") at current temperature position
  int16_t tempInt = (int16_t)currentCelsius;
  int pos = map(tempInt, lowTemp, maxTemp, 0, NUMPIXELS - 1);
  if (pos < 0) pos = 0;
  if (pos > (NUMPIXELS - 1)) pos = NUMPIXELS - 1;
  pixels.setPixelColor(pos, 255, 255, 255);

  pixels.show();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* 5x4 LED Matrix helpers (map 20-pixel strip to matrix coordinates)
========================================================================================================================
   _________            _                 _            
  | matrix |   utils  | |__   __ _  __ _| |_ ___  _ __
  |  o___o |          | '_ \ / _` |/ _` | __/ _ \| '__|
  |__/___\_|          | | | | (_| | (_| | || (_) | |   
                      |_| |_|\__,_|\__,_|\__\___/|_|   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

#define MATRIX_W 5
#define MATRIX_H 4
#define MATRIX_SERPENTINE 1   // 1 = serpentine wiring per row, 0 = progressive
#define MATRIX_ORIGIN_TOPLEFT 1 // assume pixel 0 is top-left

// Convert (x,y) -> strip index according to assumed wiring
int matrixIndex(uint8_t x, uint8_t y) {
  if (x >= MATRIX_W || y >= MATRIX_H) return -1;
  uint8_t row = y;
  uint8_t col = x;
  if (!MATRIX_ORIGIN_TOPLEFT) {
    // If origin is bottom-left, flip Y
    row = (MATRIX_H - 1) - y;
  }
  if (MATRIX_SERPENTINE) {
    if (row % 2 == 0) {
      // even row: left -> right
      return row * MATRIX_W + col;
    } else {
      // odd row: right -> left
      return row * MATRIX_W + (MATRIX_W - 1 - col);
    }
  } else {
    // progressive left -> right each row
    return row * MATRIX_W + col;
  }
}

// Set a pixel in matrix coordinates
void matrixSet(uint8_t x, uint8_t y, uint32_t color) {
  int idx = matrixIndex(x, y);
  if (idx >= 0 && idx < NUMPIXELS) {
    pixels.setPixelColor(idx, color);
  }
}

// Fill entire matrix with a color
void matrixFill(uint32_t color) {
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      matrixSet(x, y, color);
    }
  }
}

// Draw a simple binary bitmap (values 0/1) of size w*h onto the matrix
void matrixDrawBitmap(const uint8_t* bmp, uint8_t w, uint8_t h, uint32_t colorOn, uint32_t colorOff) {
  uint8_t mw = (w < MATRIX_W) ? w : MATRIX_W;
  uint8_t mh = (h < MATRIX_H) ? h : MATRIX_H;
  for (uint8_t y = 0; y < mh; y++) {
    for (uint8_t x = 0; x < mw; x++) {
      uint8_t v = bmp[y * w + x];
      matrixSet(x, y, v ? colorOn : colorOff);
    }
  }
}

// Small demo pattern: a simple smiley on 5x4
void matrixDemoSmiley() {
  // 5x4 bitmap (row-major): 1 = on, 0 = off
  const uint8_t bmp[20] = {
    0,1,0,1,0,
    0,0,0,0,0,
    1,0,0,0,1,
    0,1,1,1,0
  };
  matrixDrawBitmap(bmp, 5, 4, pixels.Color(255, 200, 0), 0);
  pixels.show();
}

// Dim all pixels by a decay amount (0..255). Higher = faster fade.
void matrixFade(uint8_t decay) {
  uint8_t keep = 255 - decay;
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    uint32_t c = pixels.getPixelColor(i);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    r = (uint16_t)r * keep / 255;
    g = (uint16_t)g * keep / 255;
    b = (uint16_t)b * keep / 255;
    pixels.setPixelColor(i, r, g, b);
  }
}

// Animate a bouncing dot across the 5x4 matrix with a fading trail.
// runtime_ms: total time to run the animation
// stepDelay_ms: delay between frames
void matrixBouncingDot(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  int8_t x = 0, y = 0;
  int8_t vx = 1, vy = 1;
  uint32_t start = millis();
  uint16_t frame = 0;

  // Start with a blank canvas
  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Fade existing pixels to create a tail
    matrixFade(48); // tweak to adjust tail length (smaller = longer tail)

    // Color cycles over time for the head
    uint8_t hue = (frame * 6) & 0xFF;
    uint32_t headColor = Wheel(hue);

    // Draw head
    matrixSet((uint8_t)x, (uint8_t)y, headColor);

    pixels.show();
    delay(stepDelay_ms);

    // Advance position
    x += vx;
    y += vy;

    // Bounce on walls
    if (x < 0) { x = 0; vx = -vx; }
    if (x >= MATRIX_W) { x = MATRIX_W - 1; vx = -vx; }
    if (y < 0) { y = 0; vy = -vy; }
    if (y >= MATRIX_H) { y = MATRIX_H - 1; vy = -vy; }

    frame++;
  }
}

// fast pin access
#define AUDIOPIN (1<<SPEAKERPIN)
#define PINLOW (PORTB&=~AUDIOPIN)
#define PINHIGH (PORTB|=AUDIOPIN)

void playSound(long freq_Hz, long duration_ms)
{
  uint16_t n;
  uint32_t delayTime_us;
  uint32_t counts;

  delayTime_us = 1000000UL / freq_Hz / 2;
  counts = duration_ms * 1000 / delayTime_us;

  for (n = 0; n < counts; n++)
  {
    PINHIGH;
    delayMicroseconds(delayTime_us);
    PINLOW;
    delayMicroseconds(delayTime_us);
  }
}


int c[] = {
  NOTE_B4, NOTE_G4, NOTE_D4, NOTE_G4, NOTE_A4, NOTE_D5, PAUSE, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_D4, NOTE_G4 
};

int pauseBetweenNotes = 80;
int noteDuration_ms = 150;

void playMart(int notes[])
{

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote <= 11; thisNote++) {

    displayBinrayValue(notes[thisNote],pixels.Color(50,0,50));
    pixels.show(); // This sends the updated pixel color to the hardware.
    playSound( notes[thisNote], noteDuration_ms);
    setColorAllPixel(0); // pixels off
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
  pixels.begin();
  pixels.setBrightness(brightness);
  
  //rainbowCycle(3,20,20);
  matrixDemoSmiley();
  delay(1000);
    for ( int i = brightness; i > 0; i--) {         
    pixels.setBrightness(i);
    pixels.show();
    delay (5);
  }
  
  setColorAllPixel(0);
  pixels.setBrightness(brightness);
  pixels.show(); // Initialize all pixels to 'off'
    
  //playMart(c);
  
  /* 
  for ( int i = 0; i < 1; i++) { 
    setColorAllPixel(Wheel(220));       // all pixels set to pink using the Wheel() function to choose a color from the spectrum 0-255
    
    for ( int i = 0; i < brightness; i++) {  
      for (int n = 0; n < NUMPIXELS; n++){       
        pixels.setPixelColor(n,i,0,i);
      }
      pixels.show();
      delay (3);
    }
    for ( int i = brightness; i > 0; i--) {         
      for (int n = 0; n < NUMPIXELS; n++){       
        pixels.setPixelColor(n,i,0,i);
      }
      pixels.show();
      delay (3);
    }
    delay(300);
  }
  delay(500);
  */
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
  // Run the bouncing dot animation continuously.
  // Each call runs for 5 seconds at ~25 FPS.
  matrixBouncingDot(5000, 40);

}
