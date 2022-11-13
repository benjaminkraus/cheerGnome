#include <Adafruit_NeoPixel.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30          /* Time ESP32 will go to sleep (in seconds) */

#define PIN A7
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);

void setup() {
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(128, 0, 0, 0));
  pixels.show();
  delay(5000);
  pixels.setPixelColor(0, pixels.Color(0, 128, 0, 0));
  pixels.show();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
}
