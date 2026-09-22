#include <Arduino.h>
#include <FastLED.h>

constexpr int NUM_LEDS = 60;
constexpr uint8_t BRIGHTNESS = 20;
static_assert(NUM_LEDS > 0, "NUM_LEDS must be at least 1");
CRGB leds[NUM_LEDS];

constexpr int PIXEL_INDEX = 7;

void setup() {
  FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // Index 7 is the eighth LED. A shorter strip remains dark.
  for (int index = 0; index < NUM_LEDS; index++) {
    if (index == PIXEL_INDEX) {
      leds[index] = CRGB(255, 0, 0);
    }
  }
  FastLED.show();
}

void loop() {
  // This static pattern is displayed once in setup().
}
