#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <Adafruit_NeoPixel.h>
#include "Secrets.h"

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30          /* Time ESP32 will go to sleep (in seconds) */

#define PIN A7
#define NUMPIXELS 1
#define CheerLightsChannelNumber 1417

RTC_DATA_ATTR uint32_t lastColor = 0;

Adafruit_NeoPixel pixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);
WiFiClient client;

String colorNames[] = {
  "red", "pink", "green", "blue",
  "cyan", "white", "warmwhite", "oldlace",
  "purple", "magenta", "yellow", "orange"};

int colorRGB[][4] = { 255,   0,   0,   0,  // "red"
                      192,   0,  32,  32,  // "pink"
                        0, 255,   0,   0,  // "green"
                        0,   0, 255,   0,  // "blue"
                        0, 128, 128,   0,  // "cyan"
                       64,  64,  64,  64,  // "white"
                        0,   0,   0, 255,  // "warmwhite"
                      128,  64,   0,  64,  // "oldlace"
                      128,   0, 128,   0,  // "purple"
                      224,   0,  32,   0,  // "magenta"
                      160,  96,   0,   0,  // "yellow"
                      224,  32,   0,   0}; // "orange"

uint32_t colorNameToInt(String colorName) {
  for (int c = 0; c < 12; c++) {
    if (colorName == colorNames[c]) {
      return pixel.Color(colorRGB[c][0], colorRGB[c][1], colorRGB[c][2], colorRGB[c][3]);
    }
  }
  return 0;
}

bool connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long time = millis();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      if ((millis() - time) > (30*1000)) {
        return false;
      }
      delay(500);
    }
  }
  return true;
}

uint32_t getCurrentColor() {
  String color = ThingSpeak.readStringField(CheerLightsChannelNumber, 1);
  int statusCode = ThingSpeak.getLastReadStatus();

  if (statusCode == 200) {
    return colorNameToInt(color);
  }
  return 0;
}

void setLEDColor(uint32_t color) {
  pixel.setPixelColor(0, color);
  pixel.show();
  lastColor = color;
}

void setup() {
  pixel.begin();
  ThingSpeak.begin(client);
}

void loop() {
  if (connectToWiFi()) {
    uint32_t currentColor = getCurrentColor();
    if (currentColor > 0) {
      setLEDColor(currentColor);
    }
  }

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
