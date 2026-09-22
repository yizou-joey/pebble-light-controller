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
  FastLED.show();
}

void loop() {
  const unsigned long now = millis();
  // Each phase rises from 0 toward 1, then abruptly resets to 0.
  // Red repeats every 1 second; green repeats every 2 seconds.
  // The phases also restart when millis() wraps after about 50 days.
  const float redPhase = (now % 1000UL) / 1000.0f;
  const float greenPhase = (now % 2000UL) / 2000.0f;
  fill_solid(leds, NUM_LEDS,
             CRGB(static_cast<uint8_t>(255 * redPhase),
                  static_cast<uint8_t>(255 * greenPhase), 0));
  FastLED.show();
}
