#include <Arduino.h>
#include <FastLED.h>

constexpr int NUM_LEDS = 40;
constexpr uint8_t BRIGHTNESS = 20;
constexpr int SENSOR_PIN = A1;  // Board label D1, GPIO3.
// Divider: 3V3 -> stretch rubber -> A1 junction -> adjustable resistance -> GND.
// Use a pot as the ground-side variable resistor: junction to wiper, one outer
// terminal to GND, and unused outer terminal tied to wiper. A fixed 1k resistor
// in series prevents zero ground-side resistance. Never power this divider at 5V.
// Tune the relaxed reading away from the ADC limits (e.g. raw 1500-2500),
// then reset and hold rubber relaxed for three seconds to capture the baseline.
// Calibration: first pixel amber. Afterwards: half the strip blue-cyan at baseline;
// +/- HALF_RANGE counts gives full/off. All red indicates an ADC rail, not stretch.
// Smaller HALF_RANGE increases sensitivity and also amplifies noise.
constexpr int HALF_RANGE = 100;  // +/-100 ADC counts spans the strip.
CRGB leds[NUM_LEDS];
float baseline = 0;
float filtered = 0;

int readAverage() {
  unsigned long sum = 0;
  for (int i = 0; i < 16; ++i) {
    sum += analogRead(SENSOR_PIN);
    delayMicroseconds(100);
  }
  return sum / 16;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[0] = CRGB(128, 64, 0);  // Calibration indicator.
  FastLED.show();
  unsigned long sum = 0;
  int samples = 0;
  const unsigned long start = millis();
  while (millis() - start < 3000) {
    sum += readAverage();
    ++samples;
    delay(10);
  }
  baseline = static_cast<float>(sum) / samples;
  filtered = baseline;
}

void loop() {
  const int raw = readAverage();
  filtered += 0.2f * (raw - filtered);
  const int delta = static_cast<int>(filtered - baseline);
  const int lit = map(constrain(delta, -HALF_RANGE, HALF_RANGE),
                      -HALF_RANGE, HALF_RANGE, 0, NUM_LEDS);
  const bool saturated = raw >= 4090 || raw <= 5;
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = saturated ? CRGB::Red
                       : ((i < lit) ? CRGB(0, 128, 255) : CRGB::Black);
  }
  FastLED.show();
  Serial.print("raw="); Serial.print(raw);
  Serial.print(" baseline="); Serial.print(baseline, 1);
  Serial.print(" delta="); Serial.print(delta);
  Serial.print(" lit="); Serial.print(lit);
  Serial.print(" rail="); Serial.println(saturated ? "YES" : "no");
  delay(50);
}
