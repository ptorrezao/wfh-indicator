#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>    
#include <FastLED.h>

#include <configs.cpp> 

#define NUM_LEDS 64
#define DATA_PIN 2

CRGB leds[NUM_LEDS];

/*
    Change the Configurations on the file configs.cpp
*/
void initializeNetwork(){
    Serial.begin(Configs.baudRate);
    WiFiManager wifiManager;
    wifiManager.autoConnect(Configs.APModeLabel);
    Serial.println("Device Connected to the Network.");
}
void TurnOnLed(uint32_t color)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
    FastLED.show();
}
void initializeLed(){
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is assumed
    for (int i = 0; i < NUM_LEDS; i++) {
        if(i>=1) { leds[i-1] = CRGB::Black; } else { leds[NUM_LEDS-1] = CRGB::Black; }
        leds[i] = CRGB::White;
        delay(35);
        FastLED.show();
    }
    TurnOnLed(CRGB::White);
    delay(500);
    TurnOnLed(CRGB::Black);
}

void setup() {
    initializeNetwork();
    initializeLed();
}
void loop() {
    Serial.println("Connected...yeey :)");
    delay(1000);
}
