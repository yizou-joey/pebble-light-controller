#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>

constexpr char AP_SSID[] = "Pebble-Test";
constexpr char AP_PASSWORD[] = "pebble123";
constexpr int NUM_LEDS = 60;
CRGB leds[NUM_LEDS];
WebServer server(80);
bool apReady = false;

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
    FastLED.clear(true);

    WiFi.mode(WIFI_AP);
    IPAddress ip(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    if (!WiFi.softAPConfig(ip, ip, subnet) ||
        !WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        Serial.println("ERROR: Wi-Fi AP startup failed. Press RESET to retry.");
        return;
    }

    server.on("/", HTTP_GET, []() {
        server.sendHeader("Cache-Control", "no-store");
        String page = "<!doctype html><html lang='zh-CN'><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1'>"
            "<title>Pebble 连接测试</title>"
            "<body style='font-family:system-ui;padding:24px;line-height:1.7'>"
            "<h1>连接成功！</h1>"
            "<p>这是 XIAO ESP32C3 上的本地网页。</p>"
            "<p>Wi-Fi：Pebble-Test<br>地址：http://192.168.4.1</p>"
            "<p>灯珠暂时保持熄灭。此热点不提供互联网连接。</p>"
            "<p>设备运行时间：";
        page += String(millis() / 1000);
        page += " 秒</p><p><a href='/'>刷新连接测试</a></p></body></html>";
        server.send(200, "text/html; charset=utf-8", page);
    });
    server.begin();
    apReady = true;
    Serial.println("Wi-Fi AP ready: Pebble-Test");
    Serial.print("Open http://");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    if (apReady) {
        server.handleClient();
    }
    delay(2);
}
