#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>

// --- Hardware Definitions ---
#define NUMPIXELS 20
#define NEOPIXELPIN 0
#define SPEAKERPIN 1
#define BUTTON_PIN A3
#define POTI_LEFT  A2
#define POTI_RIGHT A1

// --- Audio ---
volatile uint16_t phase1 = 0;
volatile uint16_t phase2 = 0;
volatile uint16_t phase_inc1 = 0;
volatile uint16_t phase_inc2 = 0;
volatile uint16_t ticks = 0;

// --- NeoPixels ---
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Helpers ---
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

// Custom delay using ISR ticks (10kHz)
void my_delay(uint16_t ms) {
    uint16_t duration = ms * 10;
    uint16_t start;
    uint8_t sreg = SREG; cli(); start = ticks; SREG = sreg;
    
    while (1) {
        uint16_t current;
        sreg = SREG; cli(); current = ticks; SREG = sreg;
        if ((uint16_t)(current - start) >= duration) break;
    }
}

void setup() {
  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  // PLL for 64MHz Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Timer1 PWM (Audio Output)
  TIMSK = 0;
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10; 
  pinMode(SPEAKERPIN, OUTPUT);

  // Timer0 10kHz Interrupt
  // Fast PWM mode with OCR0A as TOP
  TCCR0A = 3 << WGM00; // WGM01 | WGM00
  TCCR0B = 1 << WGM02 | 2 << CS00; // WGM02 | Prescaler 8
  TIMSK = 1 << OCIE0A;
  OCR0A = 199; // (16MHz / 8) / (199+1) = 10kHz

  pinMode(NEOPIXELPIN, OUTPUT);
  pixels.begin();
  pixels.setBrightness(60); // Not too bright
}

void loop() {
  // Read Pots
  int val1 = analogRead(POTI_LEFT);
  int val2 = analogRead(POTI_RIGHT);
  
  // Map to frequency increments
  // Exponential-ish mapping for better pitch control
  // Base frequency ~50Hz to ~1kHz
  
  // Simple linear mapping first:
  // phase_inc = (Freq * 65536) / 10000
  // Freq = (phase_inc * 10000) / 65536
  
  // Let's try to make it usable.
  // Val 0 -> 50Hz
  // Val 1023 -> 800Hz
  
  phase_inc1 = map(val1, 0, 1023, 200, 5000);
  phase_inc2 = map(val2, 0, 1023, 200, 5000);
  
  // Visuals
  uint8_t hue1 = map(val1, 0, 1023, 0, 255);
  uint8_t hue2 = map(val2, 0, 1023, 0, 255);
  
  // Matrix is 5x4. 
  // Let's just split left/right physically on the strip for simplicity or map to the matrix.
  // Assuming standard strip order.
  
  for(int i=0; i<NUMPIXELS; i++) {
     if (i % 2 == 0) {
         pixels.setPixelColor(i, Wheel(hue1));
     } else {
         pixels.setPixelColor(i, Wheel(hue2));
     }
  }

  // Visual beat indicator (if frequencies are close)
  if (abs(val1 - val2) < 30) {
      // Interference pattern visual?
      // Just make it bright white if tuned together
      pixels.setBrightness(120);
  } else {
      pixels.setBrightness(60);
  }
  
  pixels.show();
  my_delay(50);
}

ISR(TIMER0_COMPA_vect) {
  ticks++;
  
  phase1 += phase_inc1;
  phase2 += phase_inc2;
  
  // Sawtooth waves (0-255)
  uint8_t samp1 = phase1 >> 8;
  uint8_t samp2 = phase2 >> 8;
  
  // Mix
  OCR1A = (samp1 + samp2) >> 1;
}
