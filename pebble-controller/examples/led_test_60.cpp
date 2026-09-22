#include <Arduino.h>
#include <FastLED.h>

constexpr uint8_t LED_PIN = D10;  // XIAO D10 = GPIO10
constexpr int NUM_LEDS = 60;
constexpr uint8_t BRIGHTNESS = 20;

CRGB leds[NUM_LEDS];

void setup() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);  // 亮度范围 0～255
    FastLED.clear(true);
}

void loop() {
    // 第一阶段：红色单点依次经过 60 颗灯珠，用时约 18 秒。
    for (int i = 0; i < NUM_LEDS; ++i) {
        FastLED.clear();
        leds[i] = CRGB::Red;
        FastLED.show();
        delay(300);
    }

    FastLED.clear(true);
    delay(1000);

    // 第二阶段：全部低亮度红灯闪烁三次，每次亮、灭各 1 秒。
    for (int i = 0; i < 3; ++i) {
        fill_solid(leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        delay(1000);
        FastLED.clear(true);
        delay(1000);
    }

    delay(2000);  // 保持熄灭，然后重新开始两阶段测试。
}
