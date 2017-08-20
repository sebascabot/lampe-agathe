#include "FastLED.h"

// Pins configuration
#define LEDS_PIN 7
#define PUSH_BUTTON_PIN 4
#define ANALOG_PIN A0

// LEDS/BULBS configuration
#define NUM_LEDS 12
#define NUM_LEDS_PER_BULB 4
#define NUM_BULBS 3

#define BRIGHTNESS 100
#define FRAMES_PER_SECOND  125 // 125 frames/sec <=> 8 milli/frame

#define HOLD_DELAY 350 // In Milli second
#define MIN_HSV_VALUE 130
#define MAX_HSV_VALUE 255

#define MIN_CYCLE_SPEED 1
#define MAX_CYCLE_SPEED 100

// Define the array of leds
CRGB leds[NUM_LEDS];

uint8_t gOffset = 0; // Set via the knob (analog value)

uint8_t gCycleSpeed = MIN_CYCLE_SPEED;

uint8_t gHsvHue = 0; // Animation (always changing)
uint8_t gHsvValue = MAX_HSV_VALUE;

uint8_t gBulbHsvHue[NUM_BULBS];

enum patternEnum {
  SOLID_ONE_MIX_HSV_HUE,
  // SOLID_ONE_MIX_HSV_VALUE,
  SOLID_THREE_RANDOM_HSV_HUE,
  SOLID_THREE_MIX_HSV_HUE,
  // SOLID_THREE_MIX_BRIGHTNESS_TRANSITION_SPEED,
  // SOLID_THREE_MIX_HUE_TRANSITION_SPEED,
  RAINBOW_MIX_SPEED,
  PATTERN_COUNT
};
uint8_t gPatternIdx = PATTERN_COUNT - 1; // Init to last PatternIdx
void (*gPattern)();

const char *patternName[PATTERN_COUNT] = {
  "[0] solid 1 : Mix => HSV Hue",
  // "[1] solid 1 : Mix => HSV Value",
  "[2] solid 3 : Random => HSV Hue",
  "[3] solid 3 : Mix => HSV Hue",
  // "[3] solid 3 : Mix => Brightness Transition Speed",
  // "[4] solid 3 : Mix => Hue Transition Speed",
  "[5] rainbow : MIX => Speed"
};

boolean wasPressed = false;
boolean prestine = true;

void setup() {
  Serial.begin(9600);

  // RGB LEDS (Digital Pin - Output)
  LEDS.addLeds<WS2812, LEDS_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Potentiometer (Analog Pin - Input)
  // ...

  // Push button (Digital Pin - Input)
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

  randomSeed(analogRead(2)); // Noise on unconnected Analog Pin 2 give random

  updateGpattern();

  for (int i = 1; i < NUM_BULBS; i += 1) {
    gBulbHsvHue[i] = random(8);
  }

  gPattern = solidWhite;

  Serial.println(" @@@@@@@@@@@@@@@@@@@ REBOOT @@@@@@@@@@@@@@");
  Serial.println("Setup done");
  Serial.println("++++++++++++++++++ Going Live ++++++++++++++++++");
}

void loop() {
  static uint32_t holdStartTime = millis();
  static boolean holding = false;

  boolean isPressed = (digitalRead(PUSH_BUTTON_PIN) == LOW);

  if (isPressed && !wasPressed) { // Push Detection
    prestine = false;
    wasPressed = true;
    holdStartTime = millis();
    holding = false;

    Serial.println(" >>> Button DOWN (push) transition detected");
  } else if (isPressed && wasPressed) { // Pressed
    // Serial.println(" <P> Button in PRESSED state");

    if (millis() - holdStartTime < HOLD_DELAY) {
      //Serial.println("NO : Not holding yet ----------");
      holding = false;
    } else {
      holding = true;
      updateGpatternIdx();
      Serial.println("YES : Holding now ++++++++++");
    }

  } else if (!isPressed && wasPressed) { // Release detection
    wasPressed = false;
    Serial.println(" <<< Button UP (release) transition detected");

    if (!holding) {
      // Select Next Pattern
      gPatternIdx = (gPatternIdx + 1) % PATTERN_COUNT;
      updateGpattern();
      Serial.println("Changed pattern to:");
      Serial.println(patternName[gPatternIdx]);
    } else {
      Serial.println("Nothing to do... Was holding");
      // Do nothing, was processing pressed state function
    }

    holding = false;
  } else if (!isPressed && !wasPressed) {
    // Serial.println(" <R> Button in RELEASED state");
  } else {
    Serial.println("ERROR! Impossible state.");
  }

  EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ) {
    gPattern();
    FastLED.show();
  }

  /*
    EVERY_N_MILLISECONDS( 40 ) {
      gHsvHue += 1;
    }
  */
}

void solidBulb(uint8_t idx, uint8_t hue) {
  uint8_t bulbOffset = (idx * NUM_LEDS_PER_BULB);
  for (uint8_t i = 0; i < NUM_LEDS_PER_BULB; i += 1) {
    leds[i + bulbOffset] = CHSV(hue, 255, gHsvValue);
  }
}

boolean updateGpatternIdx() {
  uint16_t analogValue = analogRead(ANALOG_PIN); // Integer [0 .. 1023] (10 bit ADC)

  uint8_t patternIdx = map(analogValue, 0, 1024, 0, PATTERN_COUNT);

  if (patternIdx != gPatternIdx && analogValue % (1024 / PATTERN_COUNT) > 5 ) { // Magic +/- 5 is the deadband to consider a change
    gPatternIdx = patternIdx;
    Serial.println("Changed pattern to:");
    Serial.println(patternName[gPatternIdx]);
    Serial.println(analogValue);
    updateGpattern();
    return true;
  } else {
    return false;
  }
}

// Sync gOffset with our Analog Reading.
void gOffsetSync() {
  uint8_t offset = map(analogRead(ANALOG_PIN), 0, 1023, 0, 255); // 10 Bits ADC (Int [0 to 1023])

  if (gOffset != offset && abs(gOffset - offset) > 3) {
    gOffset = offset;
    Serial.println("Changed gOffset to:");
    Serial.println(gOffset);

  }
}

// Sync gHsvValue with our Analog Reading.
void gHsvValueSync() {
  uint8_t hsvValue = map(analogRead(ANALOG_PIN), 0, 1023, MIN_HSV_VALUE, 255); // 10 Bits ADC (Int [0 to 1023])

  // Magic [-3, 3] interval, is the deadband to trigger change detection
  if (gHsvValue != hsvValue && abs(gHsvValue - hsvValue) > 2) {
    gHsvValue = hsvValue;
    Serial.println("Changed gHsvValue to:");
    Serial.println(gHsvValue);
  }
}

void gHsvHueSync() {
  uint8_t hsvHue = map(analogRead(ANALOG_PIN), 0, 1023, 0, 255); // 10 Bits ADC (Int [0 to 1023])

  // Magic [-3, 3] interval, is the deadband to trigger change detection
  if (gHsvHue != hsvHue && abs(gHsvHue - hsvHue) > 2) {
    gHsvHue = hsvHue;
    Serial.println("Changed gHsvHue to:");
    Serial.println(gHsvHue);
  }
}

void gBulbHsvHueRandomSync() {
  uint8_t offset = map(analogRead(ANALOG_PIN), 0, 1023, 0, 255); // 10 Bits ADC (Int [0 to 1023])

  if (gOffset != offset && abs(gOffset - offset) > 4) {
    // Init random Bulb hue
    for (int i = 0; i < NUM_BULBS; i += 1) {
      gBulbHsvHue[i] = random8();

      //  Serial.println("Bulb #");
      //  Serial.println(i);
      //  Serial.println(gBulbHsvHue[i]);
    }
    gOffset = offset;

    Serial.println("Changed all Bulbs HSV Hue. Update gOffset to:");
    Serial.println(gOffset);
  }

}


void gCycleSpeedSync() {
  uint8_t cycleSpeed = map(analogRead(ANALOG_PIN), 0, 1023, MIN_CYCLE_SPEED, MAX_CYCLE_SPEED); // 10 Bits ADC (Int [0 to 1023])

  // Magic [-2, 2] interval, is the deadband to trigger change detection
  if (gCycleSpeed != cycleSpeed && abs(gCycleSpeed - cycleSpeed) > 2) {
    gCycleSpeed = cycleSpeed;
    Serial.println("Changed gCycleSpeed to:");
    Serial.println(gCycleSpeed);
  }
}

void gCycleHsvHue() {
  static uint32_t previous = millis();

  if (millis() - previous > gCycleSpeed) {
    previous = millis();
    gHsvHue += 1;
  }
}

void solidOneMixHsvHue() {
  gHsvHueSync();
  fill_solid(leds, NUM_LEDS, CHSV(gHsvHue, 255, 255));
}

void solidOneMixHsvValue() {
  gHsvValueSync();
  fill_solid(leds, NUM_LEDS, CHSV(gHsvHue, 255, gHsvValue));
}

void solidThreeRandomHsvHue() {
  gBulbHsvHueRandomSync();
  for (int i = 0; i < NUM_BULBS; i += 1) {
    solidBulb(i, gBulbHsvHue[i]);
  }
}

void solidThreeMixHsvHue() {
  gOffsetSync();
  for (int i = 0; i < NUM_BULBS; i += 1) {
    solidBulb(i, gBulbHsvHue[i] + gOffset);
  }
}

void solidThreeMixHsvValueTransitionSpeed() {
  fill_solid(leds, NUM_LEDS, CRGB::Red);
}

void solidThreeMixHsvHueTransitionSpeed() {
  fill_solid(leds, NUM_LEDS, CRGB::Green);
}

void rainbowMixSpeed() {
  gCycleSpeedSync();
  gCycleHsvHue();
  fill_rainbow(leds, NUM_LEDS, gHsvHue, 12);;
}

void solidWhite() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
}

void updateGpattern() {
  switch (gPatternIdx) {
    case   SOLID_ONE_MIX_HSV_HUE:
      gPattern = solidOneMixHsvHue;
      break;
/*
    case SOLID_ONE_MIX_HSV_VALUE:
      gPattern = solidOneMixHsvValue;
      break;
      */
    case SOLID_THREE_RANDOM_HSV_HUE:
      gPattern = solidThreeRandomHsvHue;
      break;
    case SOLID_THREE_MIX_HSV_HUE:
      gPattern = solidThreeMixHsvHue;
      break;
    /*
      case SOLID_THREE_MIX_HSV_VALUE_TRANSITION_SPEED:
      gPattern = solidThreeMixHsvValueTransitionSpeed;
      break;
      case SOLID_THREE_MIX_HSV_HUE_TRANSITION_SPEED:
      gPattern = solidThreeMixHsvHueTransitionSpeed;
      break;
    */
    case RAINBOW_MIX_SPEED:
      gPattern = rainbowMixSpeed;
      break;
    default:
      gPattern = solidWhite;
  }
}
