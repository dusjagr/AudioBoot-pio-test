#include <OneWire.h>
#include <Adafruit_NeoPixel.h>
#include "pitches.h"
#include <neolib.h>
#include <avr/pgmspace.h>

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library


#define ONEWIREPIN   PB4

#define hell          55 // Brightness
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

// Matrix DnB Dancer: two-frame dancer that bounces to a drum-and-bass pulse
void matrixDnBDancer(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  // Two poses (5x4), 0=off, 1=body, 2=accent
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
  uint16_t beat = 0; // increments each frame

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Bass pulse every 8 frames: brief background flash
    bool bass = ((beat & 0x07) == 0);
    if (bass) {
      // soft pink flash background
      for (uint8_t y = 0; y < H; y++) for (uint8_t x = 0; x < W; x++) matrixSet(x, y, dimColor(pixels.Color(255,60,140), 80));
    } else {
      matrixFade(64);
    }

    // Alternate pose each frame for a quick bounce
    uint8_t f = (beat & 0x01);
    const uint8_t* spr = frames[f];

    // Colors: body magenta/pink, accents aqua on bass
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

// Matrix Kanji Scroller: scroll a 10x4 pixel glyph across the 5x4 matrix
void matrixKanjiScroll(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  const uint8_t W = 5, H = 4;
  const uint8_t GW = 10; // glyph width
  // Stylized simple glyph reminiscent of "心" (heart/spirit), 10x4
  static const uint8_t PROGMEM glyph[40] = {
    // Row 0 (10 cols)
    0,1,0,0,1, 0,1,0,0,0,
    // Row 1
    1,0,1,0,1, 0,1,0,0,0,
    // Row 2
    0,1,0,1,0, 1,0,1,0,0,
    // Row 3
    0,0,1,0,0, 1,0,1,0,0
  };

  const uint32_t colOn  = pixels.Color(255, 60, 140); // pink
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
// Matrix Shoggoth: amorphous wobbling blob with blinking eyes
void matrixShoggoth(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint16_t t = 0; // time phase
  // Base colors
  const uint32_t colCore = pixels.Color(255, 60, 140);   // pink core
  const uint32_t colRim  = pixels.Color(120, 20, 60);    // magenta rim
  const uint32_t colEye  = pixels.Color(255, 255, 255);  // white
  // Local matrix geometry to avoid dependency ordering here
  const uint8_t W = 5;
  const uint8_t H = 4;

  // Eye state
  uint8_t eyeCount = 2;
  int8_t ex[3] = {1, 3, 2};
  int8_t ey[3] = {1, 2, 1};
  bool eyesOpen = true;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Smooth trailing fade instead of hard clear
    matrixFade(48);

    // Soft pulsation between ~140..235 for gentle breathing
    uint8_t pulse = (uint8_t)(140 + ((t * 9) & 0x3F));

    // Blob center drift (gentle back/forth)
    // x oscillates 1..3, y oscillates 1..2
    uint8_t cx = (uint8_t)(1 + ((t >> 3) & 0x03));
    if (cx > (W - 2)) cx = (W - 2);
    uint8_t cy = (uint8_t)(1 + ((t >> 4) & 0x01));

    // Radius wobble 1..2
    uint8_t radius = (uint8_t)(1 + ((t >> 2) & 0x01));

    // Draw blob by distance from center (inner core + two rim rings for softer edge)
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int8_t dx = (int8_t)x - (int8_t)cx;
        int8_t dy = (int8_t)y - (int8_t)cy;
        uint8_t dist = (uint8_t)(abs(dx) + abs(dy)); // manhattan for chunky look
        if (dist <= radius) {
          uint32_t c = dimColor(colCore, pulse);
          matrixSet(x, y, c);
        } else if (dist == radius + 1) {
          uint32_t c = dimColor(colRim, (uint8_t)(pulse * 2 / 5));
          matrixSet(x, y, c);
        } else if (dist == radius + 2) {
          uint32_t c = dimColor(colRim, (uint8_t)(pulse / 6));
          matrixSet(x, y, c);
        }
      }
    }
    // Occasionally change eye positions slightly around the center
    if ((t & 0x0F) == 0x00) {
      for (uint8_t i = 0; i < eyeCount; i++) {
        int8_t ox = ((int8_t)random(3)) - 1; // -1..1
        int8_t oy = ((int8_t)random(3)) - 1;
        int8_t nx = (int8_t)cx + ox;
        int8_t ny = (int8_t)cy + oy;
        if (nx >= 0 && nx < (int8_t)W) ex[i] = (uint8_t)nx;
        if (ny >= 0 && ny < (int8_t)H) ey[i] = (uint8_t)ny;
      }
    }

    // Random blink
    if ((t & 0x1F) == 0x1F) {
      eyesOpen = !eyesOpen;
    }

    // Draw eyes on top
    if (eyesOpen) {
      for (uint8_t i = 0; i < eyeCount; i++) {
        matrixSet((uint8_t)ex[i], (uint8_t)ey[i], colEye);
      }
    } else {
      // half-blink: dim eyes
      for (uint8_t i = 0; i < eyeCount; i++) {
        matrixSet((uint8_t)ex[i], (uint8_t)ey[i], dimColor(colEye, 100));
      }
    }

    pixels.show();
    delay(stepDelay_ms);
    t++;
  }
}

// Matrix Galaga Invader: a classic alien sprite that flaps and moves across
void matrixGalagaInvader(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  // Local geometry to avoid macro order issues here
  const uint8_t W = 5;
  const uint8_t H = 4;

  // 2 animation frames (5x4). Values: 0=off, 1=body, 2=eye/highlight
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
  const uint32_t colBody = pixels.Color(60, 220, 100); // alien green
  const uint32_t colEye  = pixels.Color(255, 255, 255); // white eyes

  uint32_t start = millis();
  uint8_t fidx = 0;

  // Sweep path: left -> right -> left
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // move left to right
    for (int8_t ox = -4; ox <= (int8_t)W; ox++) {
      matrixFill(0);
      const uint8_t* spr = frames[fidx];
      // draw with horizontal offset ox, vertical centered (start at y=0)
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          uint8_t v = pgm_read_byte(&spr[y * 5 + x]);
          int8_t tx = (int8_t)x + ox; // target x with offset
          if (tx < 0 || tx >= (int8_t)W) continue;
          if (v == 1) matrixSet((uint8_t)tx, y, colBody);
          else if (v == 2) matrixSet((uint8_t)tx, y, colEye);
        }
      }
      pixels.show();
      delay(stepDelay_ms);
      fidx ^= 1; // flap
      if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
    // right to left
    for (int8_t ox = W; ox >= -4; ox--) {
      matrixFill(0);
      const uint8_t* spr = frames[fidx];
      for (uint8_t y = 0; y < H; y++) {
        for (uint8_t x = 0; x < W; x++) {
          uint8_t v = pgm_read_byte(&spr[y * 5 + x]);
          int8_t tx = (int8_t)x + ox;
          if (tx < 0 || tx >= (int8_t)W) continue;
          if (v == 1) matrixSet((uint8_t)tx, y, colBody);
          else if (v == 2) matrixSet((uint8_t)tx, y, colEye);
        }
      }
      pixels.show();
      delay(stepDelay_ms);
      fidx ^= 1;
      if ((uint16_t)(millis() - start) >= runtime_ms) return;
    }
  }
}
// Matrix Rainbow Zoom: concentric rainbow "rings" that appear to zoom
void matrixRainbowZoom(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  uint16_t phase = 0; // advances to create zoom motion

  // Local geometry to avoid dependency ordering
  const uint8_t W = 5;
  const uint8_t H = 4;

  // Fixed-point geometry (x,y in 1/16th pixel units) to avoid floats
  const int16_t CX = 2 * 16 + 8; // center x at 2.5
  const int16_t CY = 1 * 16 + 8; // center y at 1.5

  while ((uint16_t)(millis() - start) < runtime_ms) {
    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {
        int16_t fx = (int16_t)x * 16 + 8;
        int16_t fy = (int16_t)y * 16 + 8;
        int16_t dx = (fx - CX);
        int16_t dy = (fy - CY);
        uint16_t manh = (uint16_t)(abs(dx) + abs(dy)); // 0..~64
        uint8_t hue = (uint8_t)((manh * 4 + phase) & 0xFF);
        uint32_t col = Wheel(hue);
        // slight dim for softer look
        col = dimColor(col, 200);
        matrixSet(x, y, col);
      }
    }
    pixels.show();
    delay(stepDelay_ms);
    phase += 9; // speed of zoom
  }
}

// Matrix Stick-Man: a little stick figure runs left->right into a wall and bonks
void matrixStickManWall(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t W = 5;
  const uint8_t H = 4;

  // Colors
  const uint32_t cMan = pixels.Color(255, 255, 255);
  const uint32_t cWall = pixels.Color(200, 40, 40);

  // 2 running frames for arms/legs
  // Relative coordinates (dx,dy) from the man origin (top-left of figure area)
  // Figure width ~3, height ~4
  const int8_t manFrame0[][2] = {
    {1,0}, // head
    {1,1}, {1,2}, // body
    {0,1}, {2,1}, // arms out
    {0,3}, {1,3}  // legs: left straight, right near body
  };
  const uint8_t len0 = sizeof(manFrame0)/sizeof(manFrame0[0]);

  const int8_t manFrame1[][2] = {
    {1,0},
    {1,1}, {1,2},
    {0,2}, {2,2}, // arms down
    {2,3}, {1,3}  // legs swapped
  };
  const uint8_t len1 = sizeof(manFrame1)/sizeof(manFrame1[0]);

  // Animation loop
  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Man origin x sweeps from off-screen left (-1) to just before wall (W-3)
    for (int8_t ox = -1; ox <= (int8_t)(W - 3); ox++) {
      // Clear and draw wall (last column)
      matrixFill(0);
      for (uint8_t wy = 0; wy < H; wy++) matrixSet(W - 1, wy, cWall);

      // Choose running frame
      bool alt = ((ox & 1) == 0);
      if (alt) {
        for (uint8_t i = 0; i < len0; i++) {
          int8_t x = ox + manFrame0[i][0];
          int8_t y = manFrame0[i][1];
          if (x >= 0 && x < (int8_t)W && y >= 0 && y < (int8_t)H) matrixSet((uint8_t)x, (uint8_t)y, cMan);
        }
      } else {
        for (uint8_t i = 0; i < len1; i++) {
          int8_t x = ox + manFrame1[i][0];
          int8_t y = manFrame1[i][1];
          if (x >= 0 && x < (int8_t)W && y >= 0 && y < (int8_t)H) matrixSet((uint8_t)x, (uint8_t)y, cMan);
        }
      }

      pixels.show();
      delay(stepDelay_ms);
    }

    // Bonk sequence at the wall
    // Flash and small scatter
    for (uint8_t k = 0; k < 2; k++) {
      matrixFill(0);
      for (uint8_t wy = 0; wy < H; wy++) matrixSet(W - 1, wy, cWall);
      // impact spark near wall center
      matrixSet(W - 2, 1, pixels.Color(255,255,255));
      matrixSet(W - 2, 2, pixels.Color(180,180,255));
      pixels.show();
      delay(60);
      matrixFade(180);
      pixels.show();
      delay(60);
    }

    // Short reset pause
    matrixFill(0);
    pixels.show();
    delay(200);
  }
}

// Matrix Lightning: vivid white/blue lightning flashes with branching and fades
void matrixLightning(uint16_t runtime_ms) {
  uint32_t start = millis();
  randomSeed(millis());

  // local geometry (avoid dependency on later macros)
  const uint8_t W = 5;
  const uint8_t H = 4;
  auto clampi = [](int v, int lo, int hi) -> int { return (v < lo) ? lo : (v > hi ? hi : v); };

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // darkness pause
    uint16_t darkMs = 120 + (random(280));
    matrixFill(0);
    pixels.show();
    delay(darkMs);

    // one storm burst = 1..3 flashes
    uint8_t flashes = 1 + (random(3));
    for (uint8_t f = 0; f < flashes; f++) {
      // choose an origin near the top
      int x = random(W);
      int y = 0;

      // draw a jaggy path downward with small branches
      uint8_t steps = H + random(2);
      matrixFill(0);
      for (uint8_t s = 0; s < steps; s++) {
        // core strike pixel
        matrixSet((uint8_t)x, (uint8_t)y, pixels.Color(255, 255, 255));
        // a few neighbor pixels for glow (blueish)
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = clampi(x + dx, 0, W - 1);
            int ny = clampi(y + dy, 0, H - 1);
            uint32_t glow = pixels.Color(80, 120, 255);
            matrixSet((uint8_t)nx, (uint8_t)ny, glow);
          }
        }
        pixels.show();
        delay(12 + random(16));

        // jitter next segment
        x += (int8_t)(random(3)) - 1; // -1,0,+1
        y += 1; if (y >= H) y = H - 1;
        x = clampi(x, 0, W - 1);
      }

      // fast flicker on/off to simulate strobe
      for (uint8_t k = 0; k < 3; k++) {
        // dim everything a bit, then spike few hot pixels
        matrixFade(200);
        for (uint8_t i = 0; i < 3; i++) {
          int rx = random(W); int ry = random(H);
          matrixSet((uint8_t)rx, (uint8_t)ry, pixels.Color(255, 255, 255));
        }
        pixels.show();
        delay(20 + random(20));
      }

      // decay to dark
      for (uint8_t d = 0; d < 4; d++) {
        matrixFade(120);
        pixels.show();
        delay(18);
      }

      // optional thunder click (very short)
      if (random(3) == 0) {
        playSound(80 + random(60), 20 + random(30));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Flag Engine: Colors, helpers, and flag definitions (5x4)
========================================================================================================================
   _________            _                 _            
  | flags |   utils   | |__   __ _  __ _| |_ ___  _ __
  |  o___o|           | '_ \ / _` |/ _` | __/ _ \| '__|
  |__/___\|           | | | | (_| | (_| | || (_) | |   
                      |_| |_|\__,_|\__,_|\__\___/|_|   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// Basic color palette
static const uint32_t COL_BLACK  = 0;
static const uint32_t COL_WHITE  = 0xFFFFFF;
static const uint32_t COL_RED    = 0xFF0000;
static const uint32_t COL_BLUE   = 0x0000FF;
static const uint32_t COL_YELLOW = 0xFFFF00;
static const uint32_t COL_GREEN  = 0x00FF00;
static const uint32_t COL_ORANGE = 0xFF8C00; // for DE gold approximation

// Helper: write a 20-element row-major array to the 5x4 matrix
void displayFlag(const uint32_t flag[20]) {
  // Use fixed 5x4 geometry to avoid dependency on MATRIX_W/H at definition site
  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      // Read color from PROGMEM
      uint32_t c = pgm_read_dword(&flag[y * 5 + x]);
      // Convert 0xRRGGBB to pixels.Color (expects separate components)
      uint8_t r = (c >> 16) & 0xFF;
      uint8_t g = (c >> 8) & 0xFF;
      uint8_t b = c & 0xFF;
      matrixSet(x, y, pixels.Color(r, g, b));
    }
  }
  pixels.show();
}

// Flags (row-major, 5 columns x 4 rows)

// Germany: black, red, gold (approx orange) -> rows: 0 black, 1 red, 2-3 gold
static const uint32_t FLAG_DE[20] PROGMEM = {
  COL_BLACK, COL_BLACK, COL_BLACK, COL_BLACK, COL_BLACK,
  COL_RED,   COL_RED,   COL_RED,   COL_RED,   COL_RED,
  COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,
  COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE
};

// France: blue, white, red vertical thirds (2-1-2 columns)
static const uint32_t FLAG_FR[20] PROGMEM = {
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED,
  COL_BLUE, COL_BLUE, COL_WHITE, COL_RED,  COL_RED
};

// Italy: green, white, red vertical thirds (2-1-2 columns)
static const uint32_t FLAG_IT[20] PROGMEM = {
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED,
  COL_GREEN, COL_GREEN, COL_WHITE, COL_RED,   COL_RED
};

// Switzerland: red field with white cross (1-pixel thick)
static const uint32_t FLAG_CH[20] PROGMEM = {
  COL_RED, COL_RED, COL_WHITE, COL_RED, COL_RED,
  COL_RED, COL_WHITE, COL_WHITE, COL_WHITE, COL_RED,
  COL_RED, COL_RED, COL_WHITE, COL_RED, COL_RED,
  COL_RED, COL_RED, COL_WHITE, COL_RED, COL_RED
};

// Sweden: blue field with yellow cross (vertical col 1, horizontal row 1)
static const uint32_t FLAG_SE[20] PROGMEM = {
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW,
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_YELLOW, COL_BLUE, COL_BLUE, COL_BLUE
};

// Ukraine: top blue (rows 0-1), bottom yellow (rows 2-3)
static const uint32_t FLAG_UA[20] PROGMEM = {
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW,
  COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW, COL_YELLOW
};

// Poland: top white (rows 0-1), bottom red (rows 2-3)
static const uint32_t FLAG_PL[20] PROGMEM = {
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED,
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED
};

// Netherlands: red, white, blue (top->bottom). Use 1 row red, 1 row white, 2 rows blue.
static const uint32_t FLAG_NL[20] PROGMEM = {
  COL_RED, COL_RED, COL_RED, COL_RED, COL_RED,
  COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE,
  COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE, COL_BLUE
};

// Cycle through flags for a given runtime, showing each for hold_ms
void matrixFlagsShow(uint16_t runtime_ms, uint16_t hold_ms) {
  const uint8_t N = 8;
  const uint32_t* flags[N] = { FLAG_DE, FLAG_FR, FLAG_IT, FLAG_CH, FLAG_SE, FLAG_UA, FLAG_PL, FLAG_NL };
  uint32_t start = millis();
  uint8_t idx = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    displayFlag(flags[idx]);
    delay(hold_ms);
    idx = (idx + 1) % N;
  }
}

// Cross-fade between two flags stored in PROGMEM
static void fadeFlags(const uint32_t* a, const uint32_t* b, uint16_t fade_ms, uint8_t steps) {
  if (steps == 0 || fade_ms == 0) return;
  uint16_t stepDelay = fade_ms / steps;
  for (uint8_t s = 1; s <= steps; s++) {
    // weight from 0..steps
    uint8_t w2 = s;
    uint8_t w1 = steps - s;
    for (uint8_t y = 0; y < 4; y++) {
      for (uint8_t x = 0; x < 5; x++) {
        uint8_t i = y * 5 + x;
        uint32_t c1 = pgm_read_dword(&a[i]);
        uint32_t c2 = pgm_read_dword(&b[i]);
        uint8_t r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
        uint8_t r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
        uint8_t r = (uint16_t)r1 * w1 / steps + (uint16_t)r2 * w2 / steps;
        uint8_t g = (uint16_t)g1 * w1 / steps + (uint16_t)g2 * w2 / steps;
        uint8_t bch = (uint16_t)b1 * w1 / steps + (uint16_t)b2 * w2 / steps;
        matrixSet(x, y, pixels.Color(r, g, bch));
      }
    }
    pixels.show();
    delay(stepDelay);
  }
}

// Show flags with cross-fade between them
void matrixFlagsShowFade(uint16_t runtime_ms, uint16_t hold_ms, uint16_t fade_ms, uint8_t steps) {
  const uint8_t N = 8;
  const uint32_t* flags[N] = { FLAG_DE, FLAG_FR, FLAG_IT, FLAG_CH, FLAG_SE, FLAG_UA, FLAG_PL, FLAG_NL };
  uint32_t start = millis();
  uint8_t idx = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    const uint32_t* cur = flags[idx];
    const uint32_t* nxt = flags[(idx + 1) % N];
    displayFlag(cur);
    delay(hold_ms);
    fadeFlags(cur, nxt, fade_ms, steps);
    idx = (idx + 1) % N;
  }
}

// Map a heat value (0..255) to a fire-like color (black -> red -> yellow -> white)
static uint32_t colorFromHeat(uint8_t heat) {
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

 

// Matrix Fire effect inspired by classic Fire2012 adapted for 5x4
void matrixFire(uint16_t runtime_ms, uint8_t cooling, uint8_t sparking, uint8_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t FW = 5; // matrix width
  const uint8_t FH = 4; // matrix height
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

    // Draw
    for (uint8_t y = 0; y < FH; y++) {
      for (uint8_t x = 0; x < FW; x++) {
        int i = idxAt(x, y);
        matrixSet(x, y, colorFromHeat(heat[i]));
      }
    }
    pixels.show();
    delay(stepDelay_ms);
  }
}



// Use neolib's setColorAllPixel(), rainbowCycle(), Wheel(), displayBinaryValue()

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
#define MATRIX_SERPENTINE 0   // 1 = serpentine wiring per row, 0 = progressive
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
    // Sadder face: brows above eyes, strong downturned mouth corners
    // Row 0: eyes
    0,1,0,1,0,
    // Row 1: subtle brows at corners
    1,0,0,0,1,
    // Row 2: mouth center
    0,0,1,0,0,
    // Row 3: mouth corners down
    1,0,0,0,1
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

// Matrix Larson Scanner: classic Cylon/Knight Rider sweep with tails
void matrixLarsonScanner(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();

  int8_t pos = 0;
  int8_t dir = 1;
  // Choose a middle-ish row for a nice horizontal sweep
  uint8_t y = (MATRIX_H > 2) ? 2 : 1; // for 4 rows -> row index 2

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Slight fade to keep visible tails
    matrixFade(40);

    uint32_t head = pixels.Color(255, 0, 0);
    uint32_t tail1 = dimColor(head, 140);
    uint32_t tail2 = dimColor(head, 60);

    // Dimmer duplicates for rows above and below, only center pixel (no tails)
    int8_t yUp = ((int8_t)y) - 1;
    int8_t yDown = ((int8_t)y) + 1;
    // Adjacent row intensity (dimmer than main head)
    uint32_t headAdj = dimColor(head, 80);
    // Opposite direction x for adjacent rows
    uint8_t oppX = (uint8_t)(MATRIX_W - 1 - pos);

    // Draw head and symmetric tails
    matrixSet((uint8_t)pos, y, head);
    if (pos - 1 >= 0) matrixSet((uint8_t)(pos - 1), y, tail1);
    if (pos + 1 < MATRIX_W) matrixSet((uint8_t)(pos + 1), y, tail1);
    if (pos - 2 >= 0) matrixSet((uint8_t)(pos - 2), y, tail2);
    if (pos + 2 < MATRIX_W) matrixSet((uint8_t)(pos + 2), y, tail2);

    // Mirror on row above (if exists): only center pixel, opposite direction
    if (yUp >= 0) {
      matrixSet(oppX, (uint8_t)yUp, headAdj);
    }

    // Mirror on row below (if exists): only center pixel, opposite direction
    if (yDown < (int8_t)MATRIX_H) {
      matrixSet(oppX, (uint8_t)yDown, headAdj);
    }

    pixels.show();
    delay(stepDelay_ms);

    // Advance and bounce at the ends
    pos += dir;
    if (pos <= 0) { pos = 0; dir = 1; }
    if (pos >= (MATRIX_W - 1)) { pos = MATRIX_W - 1; dir = -1; }
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

// Matrix Rain: random blue drops appear at top and fall down with tail fade
void matrixRain(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Fade entire matrix to create trails
    matrixFade(64);

    // Spawn a new drop at random X on the top row occasionally
    if ((millis() & 0x03) == 0) { // lightweight pseudo-random rate
      uint8_t x = (uint8_t)random(MATRIX_W);
      matrixSet(x, 0, pixels.Color(20, 60, 150));
    }

    // Move drops down by copying rows from top-1 to bottom
    // We do this by reading and writing strip indices directly for speed
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

// Matrix Twinkle: random pixels flash gently with varying colors
void matrixTwinkle(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Fade slightly so old twinkles decay
    matrixFade(40);

    // Add a few new twinkles per frame
    for (uint8_t i = 0; i < 2; i++) {
      uint8_t x = (uint8_t)random(MATRIX_W);
      uint8_t y = (uint8_t)random(MATRIX_H);
      uint8_t hue = (uint8_t)random(256);
      uint32_t c = Wheel(hue);
      // Dim to keep subtle
      c = dimColor(c, 160);
      matrixSet(x, y, c);
    }

    pixels.show();
    delay(stepDelay_ms);
  }
}

// Matrix Wipe: wipe a solid color across rows back and forth
void matrixWipe(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();

  uint8_t phase = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    uint8_t hue = (phase * 8);
    uint32_t c = Wheel(hue);

    // Sweep left->right then right->left across each row
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      if (y % 2 == 0) {
        for (uint8_t x = 0; x < MATRIX_W; x++) { matrixSet(x, y, c); pixels.show(); delay(stepDelay_ms); }
      } else {
        for (int8_t x = MATRIX_W - 1; x >= 0; x--) { matrixSet((uint8_t)x, y, c); pixels.show(); delay(stepDelay_ms); }
      }
    }

    // Clear before next color
    matrixFill(0);
    pixels.show();
    phase++;
  }
}

// Matrix Spinner: a bright dot moves around the perimeter with a fading trail
void matrixSpinner(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  // Precompute perimeter path (clockwise)
  const uint8_t pathLen = (MATRIX_W * 2) + (MATRIX_H * 2) - 4; // rectangular perimeter
  uint8_t xs[24];
  uint8_t ys[24];
  uint8_t idx = 0;
  for (uint8_t x = 0; x < MATRIX_W; x++) { xs[idx] = x; ys[idx] = 0; idx++; }
  for (uint8_t y = 1; y < MATRIX_H; y++) { xs[idx] = MATRIX_W - 1; ys[idx] = y; idx++; }
  for (int8_t x = MATRIX_W - 2; x >= 0; x--) { xs[idx] = (uint8_t)x; ys[idx] = MATRIX_H - 1; idx++; }
  for (int8_t y = MATRIX_H - 2; y > 0; y--) { xs[idx] = 0; ys[idx] = (uint8_t)y; idx++; }

  uint32_t start = millis();
  matrixFill(0);
  pixels.show();
  uint16_t step = 0;

  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48);
    uint8_t p = step % pathLen;
    uint8_t x = xs[p];
    uint8_t y = ys[p];
    uint32_t c = Wheel((step * 5) & 0xFF);
    matrixSet(x, y, c);
    pixels.show();
  delay(stepDelay_ms);
  step++;
}
}

// Matrix Pong: simple pong game demo with AI paddles and bouncing ball
void matrixPong(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  const uint8_t PADDLE_H = 2;           // 2-pixel paddles for 4-row matrix
  const uint8_t LEFT_X = 0;
  const uint8_t RIGHT_X = MATRIX_W - 1;

  // Positions
  int8_t ballX = MATRIX_W / 2;
  int8_t ballY = MATRIX_H / 2;
  int8_t vx = 1;
  int8_t vy = 1;
  int8_t lpY = 1;  // left paddle top
  int8_t rpY = 1;  // right paddle top

  // Colors
  const uint32_t colBall = pixels.Color(255, 200, 220); // soft pink-white
  const uint32_t colPaddle = pixels.Color(40, 200, 120); // teal-green

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Clear frame
    matrixFill(0);

    // Draw paddles
    for (uint8_t i = 0; i < PADDLE_H; i++) {
      int8_t ly = lpY + i; if (ly >= 0 && ly < MATRIX_H) matrixSet(LEFT_X, (uint8_t)ly, colPaddle);
      int8_t ry = rpY + i; if (ry >= 0 && ry < MATRIX_H) matrixSet(RIGHT_X, (uint8_t)ry, colPaddle);
    }

    // Draw ball
    if (ballX >= 0 && ballX < MATRIX_W && ballY >= 0 && ballY < MATRIX_H) {
      matrixSet((uint8_t)ballX, (uint8_t)ballY, colBall);
    }

    pixels.show();
    delay(stepDelay_ms);

    // Move ball
    ballX += vx;
    ballY += vy;

    // Bounce on top/bottom
    if (ballY < 0) { ballY = 0; vy = -vy; }
    if (ballY >= MATRIX_H) { ballY = MATRIX_H - 1; vy = -vy; }

    // Left paddle collision (ball moving left, at x==1 next to paddle)
    if (vx < 0 && ballX == (LEFT_X + 1)) {
      if (ballY >= lpY && ballY < (lpY + PADDLE_H)) {
        vx = 1;
        // Add a little vertical "spin" based on hit position
        int8_t hit = ballY - lpY; // 0 or 1
        vy = (hit == 0) ? -1 : 1;
      }
    }

    // Right paddle collision (ball moving right, at x==MATRIX_W-2)
    if (vx > 0 && ballX == (RIGHT_X - 1)) {
      if (ballY >= rpY && ballY < (rpY + PADDLE_H)) {
        vx = -1;
        int8_t hit = ballY - rpY;
        vy = (hit == 0) ? -1 : 1;
      }
    }

    // If ball goes past paddles, reset to center
    if (ballX < 0 || ballX >= MATRIX_W) {
      ballX = MATRIX_W / 2;
      ballY = MATRIX_H / 2;
      vx = (random(2) == 0) ? 1 : -1;
      vy = (random(2) == 0) ? 1 : -1;
    }

    // Very simple AI: move paddles toward the ball with a slight lag
    int8_t lpCenter = lpY + (PADDLE_H / 2);
    if (ballY > lpCenter) lpY++;
    else if (ballY < lpCenter) lpY--;
    if (lpY < 0) lpY = 0;
    if (lpY > (MATRIX_H - PADDLE_H)) lpY = MATRIX_H - PADDLE_H;

    int8_t rpCenter = rpY + (PADDLE_H / 2);
    if (ballY > rpCenter) rpY++;
    else if (ballY < rpCenter) rpY--;
    if (rpY < 0) rpY = 0;
    if (rpY > (MATRIX_H - PADDLE_H)) rpY = MATRIX_H - PADDLE_H;
  }
}

// Matrix Switch Toggle: shows a simple switch body with a lever that opens/closes
void matrixSwitchToggle(uint16_t runtime_ms, uint16_t frameDelay_ms) {
  // 5x4 frames; values: 0=off, 1=body, 2=lever
  const uint8_t F = 4; // number of frames
  const uint8_t frames[F][20] = {
    // Frame 0: lever left (closed)
    {
      0,1,1,1,0,
      0,1,2,1,0,
      0,1,1,1,0,
      0,0,1,0,0
    },
    // Frame 1: lever mid-left
    {
      0,1,1,1,0,
      0,1,1,2,0,
      0,1,1,1,0,
      0,0,1,0,0
    },
    // Frame 2: lever mid-right
    {
      0,1,1,1,0,
      0,2,1,1,0,
      0,1,1,1,0,
      0,0,1,0,0
    },
    // Frame 3: lever right (open)
    {
      0,1,1,1,0,
      2,1,1,1,0,
      0,1,1,1,0,
      0,0,1,0,0
    }
  };

  uint32_t start = millis();
  uint8_t idx = 0;
  int8_t dir = 1;
  const uint32_t body = pixels.Color(80, 80, 80);   // grey body
  const uint32_t leverOn = pixels.Color(255, 20, 120); // pink lever

  while ((uint16_t)(millis() - start) < runtime_ms) {
    // Draw current frame
    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        uint8_t v = frames[idx][y * MATRIX_W + x];
        if (v == 0) {
          matrixSet(x, y, 0);
        } else if (v == 1) {
          matrixSet(x, y, body);
        } else { // v == 2
          matrixSet(x, y, leverOn);
        }
      }
    }
    pixels.show();
    delay(frameDelay_ms);

    // Advance frame (ping-pong)
    idx += dir;
    if (idx == 0 || idx == (F - 1)) {
      dir = -dir;
    }
  }
}

// Matrix Comet Sweep: a bright head sweeps through all pixels with trailing fade
void matrixCometSweep(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();

  uint16_t pos = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(56);

    // Convert linear pos to matrix coordinates (row-major serpentine aware through matrixIndex)
    uint8_t y = (pos / MATRIX_W) % MATRIX_H;
    uint8_t x = pos % MATRIX_W;
    // To get a nicer path across actual wiring, map to index then set by index
    int idx = matrixIndex(x, y);
    uint32_t c = Wheel((pos * 7) & 0xFF);
    pixels.setPixelColor(idx, c);

    pixels.show();
    delay(stepDelay_ms);
    pos = (pos + 1) % (MATRIX_W * MATRIX_H);
  }
}

// Matrix Pink Spiral: vivid pink dot traces an inward spiral with a gentle trail
void matrixPinkSpiral(uint16_t runtime_ms, uint16_t stepDelay_ms) {
  // Precompute spiral order for MATRIX_W x MATRIX_H
  const uint8_t W = MATRIX_W;
  const uint8_t H = MATRIX_H;
  uint8_t xs[W * H];
  uint8_t ys[W * H];
  uint8_t count = 0;

  int left = 0, right = W - 1, top = 0, bottom = H - 1;
  while (left <= right && top <= bottom) {
    // Top row: left -> right
    for (int x = left; x <= right; x++) { xs[count] = (uint8_t)x; ys[count] = (uint8_t)top; count++; }
    // Right column: top+1 -> bottom
    for (int y = top + 1; y <= bottom; y++) { xs[count] = (uint8_t)right; ys[count] = (uint8_t)y; count++; }
    if (top < bottom) {
      // Bottom row: right-1 -> left
      for (int x = right - 1; x >= left; x--) { xs[count] = (uint8_t)x; ys[count] = (uint8_t)bottom; count++; }
    }
    if (left < right) {
      // Left column: bottom-1 -> top+1
      for (int y = bottom - 1; y > top; y--) { xs[count] = (uint8_t)left; ys[count] = (uint8_t)y; count++; }
    }
    left++; right--; top++; bottom--;
  }

  const uint32_t basePink = pixels.Color(255, 20, 120); // vivid pink
  uint32_t start = millis();
  matrixFill(0);
  pixels.show();

  uint16_t step = 0;
  while ((uint16_t)(millis() - start) < runtime_ms) {
    matrixFade(48); // gentle trail

    uint8_t idx = step % count;
    uint8_t x = xs[idx];
    uint8_t y = ys[idx];

    // Soft pulse between 160..255
    uint8_t pulse = 160 + ((step * 11) & 0x3F);
    uint32_t c = dimColor(basePink, pulse);
    matrixSet(x, y, c);

    pixels.show();
    delay(stepDelay_ms);
    step++;
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
  // Preview effect
  // Choose ONE visual by uncommenting a line below
  // Each call uses (runtime_ms, speed/stepDelay params ...)
  // Lightweight visuals:
  //matrixRainbowZoom(12000, 80);
  //matrixPinkSpiral(12000, 60);
  //matrixBouncingDot(10000, 80);
  //matrixRain(10000, 60);
  //matrixTwinkle(10000, 60);
  //matrixLarsonScanner(10000, 120);

  // Themed/new visuals:
  //matrixKanjiScroll(12000, 90);
  //matrixShoggoth(12000, 90);
  matrixDnBDancer(12000, 290);

  // Flag engine:
  //matrixFlagsShowFade(14000, 900, 350, 6);
  //matrixFlagsShow(12000, 800);

  // Heavier visuals (uncomment only if code size allows):
  //matrixSpinner(10000, 60);
  //matrixWipe(10000, 20);
  //matrixCometSweep(10000, 50);
  //matrixFire(12000, 60, 80, 40);
  //matrixLightning(10000);
  //matrixGalagaInvader(10000, 120);
  //matrixStickManWall(10000, 120);
}
