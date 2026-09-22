#include <Arduino.h>
#include <FastLED.h>
#include <math.h>

constexpr int NUM_LEDS = 60;
constexpr uint8_t BRIGHTNESS = 20;
static_assert(NUM_LEDS > 0, "NUM_LEDS must be at least 1");
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  const unsigned long now = millis();
  // Combine repeating time phases with each LED's spatial phase.
  // These ramps reset at their periods and when millis() wraps.
  const float redPhase = (now % 1000UL) / 1000.0f;
  const float greenPhase = (now % 2000UL) / 2000.0f;
  const float bluePhase = (now % 3000UL) / 3000.0f;
  for (int index = 0; index < NUM_LEDS; index++) {
    const float spatialPhase = static_cast<float>(index) / NUM_LEDS;
    leds[index] = CRGB(
        static_cast<uint8_t>(255 * fabsf(redPhase - spatialPhase)),
        static_cast<uint8_t>(255 * fabsf(greenPhase - spatialPhase)),
        static_cast<uint8_t>(255 * fabsf(bluePhase - spatialPhase)));
  }
  FastLED.show();
}
