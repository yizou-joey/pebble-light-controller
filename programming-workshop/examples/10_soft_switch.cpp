#include <Arduino.h>
#include <FastLED.h>

constexpr int NUM_LEDS = 60;
constexpr uint8_t BRIGHTNESS = 20;
static_assert(NUM_LEDS > 0, "NUM_LEDS must be at least 1");
CRGB leds[NUM_LEDS];

// The soft switch connects D3 to GND when pressed.
constexpr int SWITCH_PIN = D3;

void setup() {
  FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);
  // INPUT_PULLUP holds the pin HIGH; pressing the switch pulls it LOW.
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  const bool pressed = (digitalRead(SWITCH_PIN) == LOW);
  if (pressed) {
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  FastLED.show();
  delay(10);
}
