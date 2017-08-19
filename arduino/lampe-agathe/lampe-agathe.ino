#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS 12

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 7
#define CLOCK_PIN 13
#define PUSH_BUTTON_PIN 4

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() { 
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);
	Serial.println("resetting");
	LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
	LEDS.setBrightness(84);pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

 randomSeed(analogRead(2)); // Noise on unconnected Analog Pin 2 give random
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

bool wasPressed = false;
bool prestine = true;

void loop() { 
  static uint8_t hue = 0;

  int sensorValue = analogRead(A0);
  // Serial.println(sensorValue);  
  // Serial.println((84.0 * sensorValue) / 1024.0);  

  LEDS.setBrightness(1.0 + ((75.0 * sensorValue) / 1024.0));

  bool isPressed = (digitalRead(PUSH_BUTTON_PIN) == LOW);
  if (isPressed && !wasPressed) {
    prestine = false;
    wasPressed = true;
    Serial.println("Push button pressed!");
    
    hue = random(255);
    leds[0] = CHSV(hue, 255, 255);
    leds[1] = CHSV(hue, 255, 255);
    leds[2] = CHSV(hue, 255, 255);
    leds[3] = CHSV(hue, 255, 255);

    hue = random(255);
    leds[4] = CHSV(hue, 255, 255);
    leds[5] = CHSV(hue, 255, 255);
    leds[6] = CHSV(hue, 255, 255);
    leds[7] = CHSV(hue, 255, 255);

    hue = random(255);
    leds[8] = CHSV(hue, 255, 255);
    leds[9] = CHSV(hue, 255, 255);
    leds[10] = CHSV(hue, 255, 255);
    leds[11] = CHSV(hue, 255, 255);

    
    FastLED.show();
  }
  if(!isPressed && wasPressed) {
    wasPressed = false;
    Serial.println("Push button released!");
  }

  if (!prestine) {
    FastLED.show();
    delay(100);
    return;
  }
	
	// First slide the led in one direction
	for(int i = 0; i < NUM_LEDS; i++) {
		// Set the i'th led to red 
		leds[i] = CHSV(hue++, 255, 255);
		// Show the leds
		FastLED.show(); 
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(10);
	}
	
	// Now go in the other direction.  
	for(int i = (NUM_LEDS)-1; i >= 0; i--) {
		// Set the i'th led to red 
		leds[i] = CHSV(hue++, 255, 255);
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(10);
	}
}
