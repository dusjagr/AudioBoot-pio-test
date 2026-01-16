#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>

// --- Configuration ---
// Hardware Definitions
#define NUMPIXELS 20
#define NEOPIXELPIN 0
#define SPEAKERPIN 1
#define BUTTON_PIN A3
#define POTI_PITCH A1 // Right Pot for Pitch

// Button Constants
#define BUTTON_NONE         0
#define BUTTON_LEFT         1
#define BUTTON_RIGHT        2
#define Vbutton_releaseLevel  450
#define Vbutton_left          380
#define Vbutton_right         300
#define Vbutton_both          224
#define Vbutton_pressedLevel  Vbutton_left 

#define BUTTON_NOTPRESSED   0
#define BUTTON_PRESSED      1

// --- Waveforms ---
enum Waveform {
  WAVE_SAW = 0,
  WAVE_SQUARE,
  WAVE_TRIANGLE,
  WAVE_SINE,
  WAVE_COUNT
};

volatile uint8_t current_waveform = WAVE_SAW;

// --- Synthesis Variables ---
volatile uint16_t phase = 0;
volatile uint16_t phase_increment = 0;
volatile uint16_t ticks = 0;

// --- NeoPixel ---
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// --- Sine Table ---
const byte sine256[] PROGMEM = {
  0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 128,
  128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246,
  248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211,
  208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65,
  62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0
};

// --- Note Frequency Table (MIDI 24 (C1) to MIDI 96 (C7)) ---
// Phase increments for 10kHz sample rate
// Formula: freq * 65536 / 10000
// MIDI 24 (32.7Hz) -> 214
// MIDI 96 (2093Hz) -> 13717
const uint16_t note_increments[] PROGMEM = {
  214, 227, 241, 255, 270, 286, 303, 321, 340, 361, 382, 405, // Octave 1
  429, 455, 482, 510, 541, 573, 607, 643, 681, 722, 764, 810, // Octave 2
  858, 909, 963, 1021, 1081, 1146, 1214, 1286, 1363, 1444, 1530, 1621, // Octave 3 (Middle C is 4th here, MIDI 60)
  1717, 1819, 1927, 2042, 2163, 2292, 2428, 2573, 2726, 2888, 3060, 3242, // Octave 4
  3434, 3639, 3855, 4084, 4327, 4584, 4857, 5146, 5452, 5776, 6119, 6483, // Octave 5
  6869, 7277, 7710, 8169, 8654, 9169, 9714, 10292, 10904, 11552, 12239, 12967, // Octave 6
  13738 // C7
};
#define MIN_NOTE 24
#define MAX_NOTE 96
#define NOTE_COUNT (MAX_NOTE - MIN_NOTE + 1)

// --- Helper Functions ---
uint8_t wasButtonPressed() {
  static uint8_t buttonPressed = false;
  static uint8_t buttonState = 0;
  static uint8_t buttonValue = BUTTON_NONE;
  static uint8_t buttonMaxValue = 0;
  uint8_t buttonReturnValue = BUTTON_NONE;
  uint16_t pinVoltage = analogRead(BUTTON_PIN);
  
  if (pinVoltage > Vbutton_releaseLevel) buttonPressed = false;
  if (pinVoltage < Vbutton_pressedLevel) buttonPressed = true;
  
  switch (buttonState) {
    case BUTTON_NOTPRESSED:
      buttonMaxValue = 0;
      if (buttonPressed) buttonState = BUTTON_PRESSED;
      break;
    case BUTTON_PRESSED:
      if (buttonPressed) {
        buttonValue = BUTTON_NONE;
        if (pinVoltage < Vbutton_both) buttonValue = BUTTON_LEFT + BUTTON_RIGHT;
        else if (pinVoltage < Vbutton_right) buttonValue = BUTTON_RIGHT;
        else if (pinVoltage < Vbutton_left) buttonValue = BUTTON_LEFT;
        if (buttonValue > buttonMaxValue) buttonMaxValue = buttonValue;
      } else {
        buttonState = BUTTON_NOTPRESSED;
        buttonReturnValue = buttonMaxValue;
      }
      break;
  }
  return buttonReturnValue;
}

// Custom delay
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
  TCCR0A = 3 << WGM00; 
  TCCR0B = 1 << WGM02 | 2 << CS00; 
  TIMSK = 1 << OCIE0A;
  OCR0A = 199; 

  pinMode(NEOPIXELPIN, OUTPUT);
  pixels.begin();
  pixels.setBrightness(100);
  
  // Initial Visual
  pixels.fill(pixels.Color(255, 0, 0)); // Start with SAW (Red)
  pixels.show();
}

void loop() {
  // Read Pitch Pot
  uint16_t rawVal = analogRead(POTI_PITCH);
  
  // EMA Smoothing (Factor 1/16)
  static uint32_t accumulator = 0;
  static bool first_run = true;
  
  if (first_run) {
      accumulator = (uint32_t)rawVal << 4;
      first_run = false;
  }
  
  accumulator = accumulator - (accumulator >> 4) + rawVal;
  uint16_t potVal = accumulator >> 4;
  
  // Map Pot (0-1023) to Note Index (0 to NOTE_COUNT-1)
  // We want to span the full range
  uint8_t noteIndex = map(potVal, 0, 1023, 0, NOTE_COUNT - 1);
  
  // Interpolation for smooth pitch
  // Each note covers approx 1023/NOTE_COUNT ADC steps (~14 steps per semitone)
  // We can just step to semitones for "1V/Oct" feel or interpolate.
  // Let's do semitone quantization to be strictly "musical" if it's a VCO tracking pitch.
  // Actually, analog VCOs are smooth. Let's stick to the nearest semitone for stability, 
  // or implement fine tuning if requested. For now: Semitone quantization.
  
  phase_increment = pgm_read_word(&note_increments[noteIndex]);
  
  // Check Buttons
  uint8_t btn = wasButtonPressed();
  if (btn != BUTTON_NONE) {
      current_waveform++;
      if (current_waveform >= WAVE_COUNT) current_waveform = 0;
      
      // Update Visuals based on Waveform
      uint32_t color = 0;
      switch(current_waveform) {
          case WAVE_SAW: color = pixels.Color(255, 0, 0); break; // Red
          case WAVE_SQUARE: color = pixels.Color(0, 0, 255); break; // Blue
          case WAVE_TRIANGLE: color = pixels.Color(0, 255, 0); break; // Green
          case WAVE_SINE: color = pixels.Color(255, 0, 255); break; // Purple
      }
      pixels.fill(color);
      pixels.show();
  }
  
  // Small delay to debounce/limit update rate slightly (ADC is noisy)
  my_delay(10);
}

ISR(TIMER0_COMPA_vect) {
  ticks++;
  phase += phase_increment;
  
  uint8_t sample = 0;
  uint8_t p_high = phase >> 8;
  
  switch(current_waveform) {
      case WAVE_SAW:
          sample = p_high;
          break;
          
      case WAVE_SQUARE:
          sample = (p_high > 127) ? 255 : 0;
          break;
          
      case WAVE_TRIANGLE:
          // 0 -> 127 -> 255 -> 127 -> 0
          // If phase bit 15 is 0 (0-32767), we go up.
          // If phase bit 15 is 1 (32768-65535), we go down.
          if (phase & 0x8000) {
              // Down
              sample = (uint8_t)((65535 - phase) >> 7); // Scale to 0-255 roughly
              // (32767 >> 7) is 255.
          } else {
              // Up
              sample = (uint8_t)(phase >> 7);
          }
          // Shift result to ensure full range 0-255? 
          // 32768 >> 7 = 256. Clip?
          // Let's use simplified:
          // sample = (phase & 0x8000) ? (~phase >> 7) : (phase >> 7); // Close enough
           sample = (phase & 0x8000) ? ((uint8_t)(~(phase >> 7))) : ((uint8_t)(phase >> 7));
           // Actually triangular needs shift by 1 to multiply by 2 slope?
           // 0 -> 32768 (Half cycle). Output 0 -> 255.
           // So (phase << 1) >> 8 = phase >> 7.
          break;
          
      case WAVE_SINE:
          sample = pgm_read_byte(&sine256[p_high]);
          break;
  }
  
  OCR1A = sample;
}
