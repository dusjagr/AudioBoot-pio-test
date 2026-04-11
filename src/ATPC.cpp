#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/power.h>

// --- Configuration ---
#define LED_PIN 0
#define SPEAKERPIN 1
#define BUTTON_PIN A3
#define POTI_RIGHT A1 // Param 1 (Normal) / 3 (Shift)
#define POTI_LEFT  A2 // Param 2 (Normal) / 4 (Shift)

// Button Constants
#define BUTTON_NONE         0
#define BUTTON_LEFT         1
#define BUTTON_RIGHT        2
#define BUTTON_NOTPRESSED   0
#define BUTTON_PRESSED      1
#define Vbutton_releaseLevel  450
#define Vbutton_left          380
#define Vbutton_right         300
#define Vbutton_both          224
#define Vbutton_pressedLevel  Vbutton_left 

// --- Variables ---
// 4 Virtual Pot Values (0-1023)
// p1: Freq, p2: Base PW, p3: Sweep Start PW, p4: Duration
uint16_t vPot1 = 512; 
uint16_t vPot2 = 512; 
uint16_t vPot3 = 0;   // Start with distinct value for sweep effect
uint16_t vPot4 = 0;   // Start with slow sweep

// ATPC Variables
volatile unsigned int time1 = 100;                        // 1st oscillator
volatile unsigned int pulseWidth = 100; 
volatile unsigned int pulseWidthTarget = 100;             // 2 pulse widths
volatile unsigned int duration = 10;                      // Time to reach target pulse width
volatile unsigned int tick1 = 0;
volatile unsigned int tick2 = 0;
volatile unsigned int tick3 = 0;                  // Cycle counters
volatile unsigned long ticks = 0;                 // Monotonic ticker for delay
volatile bool pulseStart = false;
bool startUp = true;

// Latch/Soft Takeover State
bool isShiftMode = false;
bool latchedRight = false;
bool latchedLeft = false;

// --- Helpers ---
// Custom delay since we broke Timer0 (millis)
void my_delay(uint16_t ms) {
    // ISR is 40kHz. 1ms = 40 ticks.
    unsigned long duration = (unsigned long)ms * 40;
    unsigned long start;
    uint8_t sreg = SREG; cli(); start = ticks; SREG = sreg;
    while (1) {
        unsigned long current;
        sreg = SREG; cli(); current = ticks; SREG = sreg;
        if (current - start >= duration) break;
    }
}

void flashLED() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle
    my_delay(50);
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle back
    my_delay(50);
}

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

void setup() {
    #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
    #endif
    
    // Timer setup
    PLLCSR = 1 << PCKE | 1 << PLLE;
    TIMSK = 0;
    TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10; 
    pinMode(SPEAKERPIN, OUTPUT);

    TCCR0A = 3 << WGM00; 
    TCCR0B = 1 << WGM02 | 2 << CS00; 
    TIMSK = 1 << OCIE0A;
    OCR0A = 49; 

    pinMode(LED_PIN, OUTPUT);
    
    // Blink LED to confirm startup
    for(int i=0; i<3; i++) {
        digitalWrite(LED_PIN, HIGH);
        my_delay(100);
        digitalWrite(LED_PIN, LOW);
        my_delay(100);
    }
}

void loop() {
    uint8_t clickedBtn = wasButtonPressed(); // Returns button on RELEASE
    
    // DEBUG: Flash LED on any button release
    if (clickedBtn != BUTTON_NONE) {
        flashLED();
    }

    uint16_t pRight = analogRead(POTI_RIGHT);
    uint16_t pLeft = analogRead(POTI_LEFT);

    // Toggle Mode Logic (Right Button Click)
    if (clickedBtn == BUTTON_RIGHT || clickedBtn == (BUTTON_LEFT + BUTTON_RIGHT)) {
        isShiftMode = !isShiftMode;
        latchedRight = false;
        latchedLeft = false;
    }
    
    // Update Visuals (Shift Mode = LED ON)
    digitalWrite(LED_PIN, isShiftMode ? HIGH : LOW);

    // Select Targets
    uint16_t* targetRight = isShiftMode ? &vPot3 : &vPot1;
    uint16_t* targetLeft = isShiftMode ? &vPot4 : &vPot2;

    // Right Pot Latch Logic
    if (!latchedRight) {
        if (abs((int)pRight - (int)*targetRight) < 80) {
            latchedRight = true;
            flashLED();
        }
    }
    if (latchedRight) {
        *targetRight = pRight;
    }

    // Left Pot Latch Logic
    if (!latchedLeft) {
        if (abs((int)pLeft - (int)*targetLeft) < 80) {
            latchedLeft = true;
            flashLED();
        }
    }
    if (latchedLeft) {
        *targetLeft = pLeft;
    }
    
    // Sync/Trigger Logic: Left Button Hold
    // Trigger only if voltage is in Left Button range (avoiding Right/Toggle button)
    uint16_t btnRaw = analogRead(BUTTON_PIN);
    bool trigger = (btnRaw > Vbutton_right && btnRaw < Vbutton_left);

    
    // Update ATPC vars
    
    // time1: Frequency. Lower is higher pitch.
    // Original: 134 - (analogRead(pot1) >> 3)
    int val1 = 134 - (vPot1 >> 3);
    time1 = (val1 < 1) ? 1 : val1;

    // pulseWidthTarget: Base Pulse Width
    // Original: 518 - (analogRead(pot2) >> 1)
    pulseWidthTarget = 518 - (vPot2 >> 1); 
    
    // Trigger Sweep
    // If there is a trigger signal and sweep is ended (pulse width = target pulse width).
    if (trigger && (pulseWidth == pulseWidthTarget)) {
         // Original: 518 - (analogRead(pot3) >> 1)
         pulseWidth = 518 - (vPot3 >> 1); 
         tick3 = 0;                       
    }
    
    // duration: Speed of sweep
    // Original: (analogRead(pot4) - 512) >> 1
    // This relies on negative results casting to large uint16_t values for SLOW sweeps.
    int16_t durRaw = ((int16_t)vPot4 - 512) >> 1;
    
    // Prevent 0 (would cause ISR crash/hang in modulo-like logic if unchecked, or just instant)
    if (durRaw == 0) {
        duration = 1; 
    } else {
        duration = (uint16_t)durRaw;
    }

    if (startUp) {
        pulseWidth = pulseWidthTarget;
        startUp = false;
    }
}

// Interrupt Service Routine called 40000 times a second
ISR(TIMER0_COMPA_vect) {
  ticks++; // Monotonic clock for my_delay
  
  // Increment tickers
  tick1 += 1;
  tick2 += 1;
  tick3 += 1;

  if (tick1 % time1 == 0 && !pulseStart) {        // If counter1 is multiple of oscillator1 and no pulse started before
    OCR1A = 255;                                  // Set square wave to top
    pulseStart = true;                            // Remember that pulse is started
    tick1 = 0;
    tick2 = 1;                                    // Start 2nd ticker
  }

  //Step closer to target pulseWidth if duration ticker is a multiple of duration time
  if (tick3 % duration == 0) {
    if (pulseWidth < pulseWidthTarget) {          // either count one step up
      pulseWidth += 1;
    } else if (pulseWidth > pulseWidthTarget) {   // or one step down
      pulseWidth -= 1;
    }
  }

  if (tick2 % pulseWidth == 0)                    // If tick2 is multiple of pulseWidth
  {
    OCR1A = 0;                                    // Square wave to bottom
    pulseStart = false;                           // Wait for next turn
  }
}
