#include <Arduino.h>
#include <FastLED.h>

constexpr int NUM_LEDS = 60;
constexpr uint8_t BRIGHTNESS = 20;
static_assert(NUM_LEDS > 0, "NUM_LEDS must be at least 1");
CRGB leds[NUM_LEDS];

constexpr unsigned long STEP_INTERVAL_MS = 100;
int position = 0;
unsigned long previousStepMs = 0;

void setup() {
  FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[position] = CRGB(255, 0, 0);
  FastLED.show();
  previousStepMs = millis();
}

void loop() {
  const unsigned long now = millis();
  // Unsigned subtraction also works when millis() wraps around.
  if (now - previousStepMs < STEP_INTERVAL_MS) {
    return;
  }
  previousStepMs = now;

  position = (position + 1) % NUM_LEDS;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[position] = CRGB(255, 0, 0);
  FastLED.show();
}
