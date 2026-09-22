#include <Arduino.h>
#include <FastLED.h>

constexpr int NUM_LEDS = 60;
constexpr uint8_t BRIGHTNESS = 20;
static_assert(NUM_LEDS > 0, "NUM_LEDS must be at least 1");
CRGB leds[NUM_LEDS];

constexpr int START_INDEX = 0;
constexpr int BLOCK_LENGTH = 5;

void setup() {
  FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int index = 0; index < NUM_LEDS; index++) {
    if (index >= START_INDEX && index - START_INDEX < BLOCK_LENGTH) {
      leds[index] = CRGB(255, 0, 0);
    }
  }
  FastLED.show();
}

void loop() {
  // This static pattern is displayed once in setup().
}
