#include <Arduino.h>
#include <FastLED.h>

constexpr int NUM_LEDS = 60;
constexpr uint8_t BRIGHTNESS = 20;
static_assert(NUM_LEDS > 0, "NUM_LEDS must be at least 1");
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int index = 0; index < NUM_LEDS; index++) {
    // Assign channels independently so overlapping regions mix colors.
    if (index < 5) {
      leds[index].r = 255;
    }
    if (index > 3) {
      leds[index].g = 255;
    }
    leds[index].b = static_cast<uint8_t>(255.0f * index / NUM_LEDS);
  }
  FastLED.show();
}

void loop() {
  // This static pattern is displayed once in setup().
}
