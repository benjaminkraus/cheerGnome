#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <Adafruit_NeoPixel.h>
#include "Secrets.h"

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds. */
#define TIME_TO_SLEEP  30          /* Time ESP32 will go to sleep (in seconds). */
#define BATTERY_REPORT_RATE 10     /* Report battery voltage every N sleep cycles. */
#define ERROR_REPORT_RATE 6        /* How many times an error occurs before blinking the light. */

#define PIN A7
#define NUMPIXELS 1
#define CheerLightsChannelNumber 1417

enum errorCodes {
  ERROR_NONE = 0,
  ERROR_WIFI = 1,
  ERROR_COLOR = 2
};

RTC_DATA_ATTR uint32_t lastColor = 4294967295;
RTC_DATA_ATTR uint8_t bootCount = BATTERY_REPORT_RATE;
RTC_DATA_ATTR uint8_t errorCode = ERROR_NONE;
RTC_DATA_ATTR uint8_t errorCount = ERROR_REPORT_RATE;

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

uint32_t getErrorColor() {
  if (errorCode == ERROR_WIFI) {
    return colorNameToInt("red");
  } else if (errorCode == ERROR_COLOR) {
    return colorNameToInt("green");
  } else {
    return 0;
  }
}

unsigned long connectionWaitTime() {
  if (errorCode == ERROR_WIFI) {
    return 10;
  } else {
    return 30;
  }
}

bool connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long time = millis();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      if ((millis() - time) > (connectionWaitTime()*1000)) {
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
    lastColor = colorNameToInt(color);
    return lastColor;
  }
  return 0;
}

void setLEDColor(uint32_t color) {
  pixel.setPixelColor(0, color);
  pixel.show();
}

float getBatteryVoltage() {
  int value = analogReadMilliVolts(BATT_MONITOR);
  return value * 2.0/1000.0;
}

void sendBatteryVoltageToThingSpeak() {
  unsigned long channel = THINGSPEAK_BATTERY_CHANNEL_NUMBER;
  const char * APIKey = THINGSPEAK_BATTERY_CHANNEL_APIKEY;
  int status = ThingSpeak.writeField(channel, 1, getBatteryVoltage(), APIKey);
  if (status == 200) {
    bootCount = 0;
  }
}

void setup() {
  ++bootCount;
  pixel.begin();
  ThingSpeak.begin(client);
}

void loop() {
  uint64_t sleepTime = TIME_TO_SLEEP;

  if (connectToWiFi()) {
    uint32_t currentColor = getCurrentColor();
    if (currentColor > 0) {
      setLEDColor(currentColor);
      errorCode = ERROR_NONE;
    } else {
      errorCode = ERROR_COLOR;
    }
    if (bootCount > BATTERY_REPORT_RATE) {
      sendBatteryVoltageToThingSpeak();
    }
  } else {
    errorCode = ERROR_WIFI;
  }

  if (errorCode && errorCount >= ERROR_REPORT_RATE) {
    uint32_t errorColor = getErrorColor();
    setLEDColor(0);
    delay(1000);
    setLEDColor(getErrorColor());
    delay(1000);
    setLEDColor(0);
    delay(1000);
    setLEDColor(getErrorColor());
    delay(1000);
    setLEDColor(0);
    delay(1000);
    setLEDColor(lastColor);
  }

  if (errorCode) {
    ++errorCount;
  } else {
    errorCount = 0;
  }

  esp_sleep_enable_timer_wakeup(sleepTime * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
