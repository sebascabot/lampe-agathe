#include "FastLED.h"

// Pins configuration
#define LEDS_PIN 7
#define PUSH_BUTTON_PIN 4
#define ANALOG_PIN A0

// LEDS/BULBS configuration
#define NUM_LEDS 12
#define NUM_LEDS_PER_BULB 4
#define NUM_BULBS 3

#define BRIGHTNESS 50
#define FRAMES_PER_SECOND  125 // 125 frames/sec <=> 8 milli/frame

#define HOLD_DELAY 350 // In Milli second

// Define the array of leds
CRGB leds[NUM_LEDS];

uint8_t gOffset = 0; // Set via the knob (analog value)
uint8_t gHue = 0; // Animation (always changing)
uint8_t gBulbsHue[NUM_BULBS];

enum patternEnum {
  SOLID_ONE_MIX_HUE,
  SOLID_ONE_MIX_BRIGHTNESS,
  SOLID_THREE_MIX_HUE,
  SOLID_THREE_MIX_BRIGHTNESS_TRANSITION_SPEED,
  SOLID_THREE_MIX_HUE_TRANSITION_SPEED,
  RAINBOW_MIX_SPEED,
  PATTERN_COUNT
};
uint8_t gPatternIdx = SOLID_ONE_MIX_HUE;
void (*gPattern)();


const char *patternName[PATTERN_COUNT] = {
  "[0] solid 1 : Mix => Hue",
  "[1] solid 1 : Mix => Brightness",
  "[2] solid 3 : Mix => Hue",
  "[3] solid 3 : Mix => Brightness Transition Speed",
  "[4] solid 3 : Mix => Hue Transition Speed",
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
    gBulbsHue[i] = random(8);
  }

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
    updateGoffset();
  } else {
    Serial.println("ERROR! Impossible state.");
  }

  EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ) {
    gPattern();
    //solidOneMixHue();
    FastLED.show();
  }

  EVERY_N_MILLISECONDS( 40 ) {
    gHue += 1;
  }
}

void solidBulb(uint8_t idx, uint8_t hue) {
  uint8_t bulbOffset = (idx * NUM_LEDS_PER_BULB);
  for (uint8_t i = 0; i < NUM_LEDS_PER_BULB; i += 1) {
    leds[i + bulbOffset] = CHSV(hue, 255, 255);
  }
}

boolean updateGpatternIdx() {
  uint16_t analogValue = analogRead(ANALOG_PIN); // Integer [0 .. 1023] (10 bit ADC)

  uint8_t patternIdx = map(analogValue, 0, 1024, 0, PATTERN_COUNT);

  if (patternIdx != gPatternIdx && analogValue % (1024 / PATTERN_COUNT) > 5 ) {
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

void updateGoffset() {
  uint8_t offset = map(analogRead(ANALOG_PIN), 0, 1023, 0, 255); // 10 Bits ADC (Int [0 to 1023])

  if (gOffset != offset && abs(gOffset - offset) > 3) {
    gOffset = offset;
    Serial.println("Changed offset to:");
    Serial.println(offset);

  }
}

void solidOneMixHue() {
  fill_solid(leds, NUM_LEDS, CHSV(gOffset, 255, 255));

  if (gBulbsHue[0] != gOffset) {
    gBulbsHue[0] = gOffset;
    for (int i = 1; i < NUM_BULBS; i += 1) {
      Serial.println("Bulb #");
      Serial.println(i);
      gBulbsHue[i] = random8();
      Serial.println(gBulbsHue[i]);
    }
  }
}

void solidOneMixBrightness() {
  fill_solid(leds, NUM_LEDS, CHSV(gBulbsHue[0], 255, gOffset));
}

void solidThreeMixHue() {
  for (int i = 0; i < NUM_BULBS; i += 1) {
    solidBulb(i, gBulbsHue[i] + gOffset);
  }
}

void solidThreeMixBrightnessTransitionSpeed() {
  fill_solid(leds, NUM_LEDS, CRGB::Red);
}

void solidThreeMixHueTransitionSpeed() {
  fill_solid(leds, NUM_LEDS, CRGB::Green);
}

void rainbowMixSpeed() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);;
}

void solidWhite() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
}

void updateGpattern() {
  switch (gPatternIdx) {
    case   SOLID_ONE_MIX_HUE:
      gPattern = solidOneMixHue;
      break;
    case SOLID_ONE_MIX_BRIGHTNESS:
      gPattern = solidOneMixBrightness;
      break;
    case SOLID_THREE_MIX_HUE:
      gPattern = solidThreeMixHue;
      break;
    case SOLID_THREE_MIX_BRIGHTNESS_TRANSITION_SPEED:
      gPattern = solidThreeMixBrightnessTransitionSpeed;
      break;
    case SOLID_THREE_MIX_HUE_TRANSITION_SPEED:
      gPattern = solidThreeMixHueTransitionSpeed;
      break;
    case RAINBOW_MIX_SPEED:
      gPattern = rainbowMixSpeed;
      break;
    default:
      gPattern = solidWhite;
  }
}
