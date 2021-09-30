#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>    
#include <FastLED.h>
#include <Wire.h>

#define NUM_LEDS 64
#define DATA_PIN 2
#define BRIGHTNESS 200

#define BAUDRATE 115200
#define APModeLabel "AutoConnectAP"

CRGB leds[NUM_LEDS];


// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

// mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion from Waveshare SPI e-Paper to generic ESP8266
// BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
// BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

// mapping suggestion for Arduino MEGA
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 53, CLK -> 52, DIN -> 51

// mapping suggestion for Arduino DUE
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 77, CLK -> 76, DIN -> 75
// SPI pins are also on 6 pin 2x3 SPI header

// include library, include base class, make path known
#include <GxEPD.h>

// select the display class to use, only one
//#include <GxGDEP015OC1/GxGDEP015OC1.h>    // 1.54" b/w
//#include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w
//#include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
//#include <GxGDEW0154Z17/GxGDEW0154Z17.h>  // 1.54" b/w/r 152x152
//#include <GxGDEW0213I5F/GxGDEW0213I5F.h>  // 2.13" b/w 104x212 flexible
//#include <GxGDE0213B1/GxGDE0213B1.h>      // 2.13" b/w
//#include <GxGDEH0213B72/GxGDEH0213B72.h>  // 2.13" b/w new panel
//#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w newer panel
//#include <GxGDEW0213Z16/GxGDEW0213Z16.h>  // 2.13" b/w/r
//#include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w
//#include <GxGDEW029T5/GxGDEW029T5.h>      // 2.9" b/w IL0373
#include <GxGDEM029T94/GxGDEM029T94.h>      // 2.9" b/w
//#include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
//#include <GxGDEW026T0/GxGDEW026T0.h>      // 2.6" b/w
//#include <GxGDEW027C44/GxGDEW027C44.h>    // 2.7" b/w/r
//#include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w
//#include <GxGDEW0371W7/GxGDEW0371W7.h>    // 3.7" b/w
//#include <GxGDEW042T2/GxGDEW042T2.h>      // 4.2" b/w
//#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
//#include <GxGDEW0583T7/GxGDEW0583T7.h>    // 5.83" b/w
//#include <GxGDEW075T8/GxGDEW075T8.h>      // 7.5" b/w
//#include <GxGDEW075T7/GxGDEW075T7.h>      // 7.5" b/w 800x480
//#include <GxGDEW075Z09/GxGDEW075Z09.h>    // 7.5" b/w/r
//#include <GxGDEW075Z08/GxGDEW075Z08.h>    // 7.5" b/w/r 800x480


// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>


#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>


/*
    Change the Configurations on the file configs.cpp
*/
void turnLedToColor(uint32_t color)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
    FastLED.show();
}
void initializeNetwork(){
    Serial.begin(BAUDRATE);
    WiFiManager wifiManager;
    wifiManager.autoConnect(APModeLabel);
    Serial.println("Device Connected to the Network.");
    
    turnLedToColor(CRGB::White);
    delay(500);
    turnLedToColor(CRGB::Black);
}
void initializeLed(){
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness( BRIGHTNESS );
    
    for (int i = 0; i < NUM_LEDS; i++) {
        if(i>=1) { leds[i-1] = CRGB::Black; } else { leds[NUM_LEDS-1] = CRGB::Black; }
        leds[i] = CRGB::White;
        delay(35);
        FastLED.show();
    }
    turnLedToColor(CRGB::Black);

}
void setup() {
    initializeLed();
    initializeNetwork();
}
void loop() {
    Serial.println("Connected...yeey :)");
    delay(1000);
}
