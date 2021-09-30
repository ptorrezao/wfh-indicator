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

#include <GxEPD.h>
#include <GxGDEM029T94/GxGDEM029T94.cpp>      // 2.9" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include "BitmapGraphics.h"
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include<string>  

GxIO_Class io(SPI, SS, 17, 16); 
GxEPD_Class display(io, 16, 4); 

const char* name = "FreeMono12pt7b";
const GFXfont* f = &FreeMono12pt7b;
const GFXfont* f9 = &FreeMono9pt7b;
const GFXfont* f9b = &FreeMonoBold9pt7b;
const GFXfont* f24b = &FreeMonoBold24pt7b;
/*
    Change the Configurations on the file configs.cpp
*/

void SetText(const String bold, const String text){
    display.setFont(f9b);
    display.print(bold);
    display.setFont(f9);
    display.println(text);
}
void turnLedToColor(uint32_t color)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
    FastLED.show();
}
void cleanScreen(){
    display.init();
    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);
    display.update();
    display.setTextColor(GxEPD_BLACK);
    display.setFont(f);
    display.setCursor(0, 15);
    display.println("Initializing...");
    display.update();
}    
void initializeScreen(const String state, const String user){
    display.setRotation(1);
    display.setCursor(0, 15);
    display.fillScreen(GxEPD_WHITE);
    display.update();
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 15);
    SetText("User:" , user);    
    display.setCursor(80, 15);
    display.setFont(f24b);
    display.println();
    display.println(" "+state);
    display.update();
    delay(3000);
    display.setRotation(0);
}
void initializeNetwork(){
    Serial.begin(BAUDRATE);
    WiFiManager wifiManager;
    wifiManager.autoConnect(APModeLabel);
    Serial.println("Device Connected to the Network.");
    
    display.setFont(f9);
    display.fillScreen(GxEPD_WHITE);
    display.update();
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 15);

    SetText("SSID:",wifiManager.getWiFiSSID());
    SetText("IP:",WiFi.localIP().toString());
    SetText("Mac:",WiFi.macAddress());

    display.update();
   
    turnLedToColor(CRGB::White);
    FastLED.setBrightness( 100 );
    delay(500);
    FastLED.setBrightness( BRIGHTNESS );
    turnLedToColor(CRGB::Black);

    delay(1000);
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
    cleanScreen();
    initializeLed();
    initializeNetwork();
    initializeScreen("WFH","PTorrezao");
}
void loop() {
    Serial.println("Connected...yeey :)");
    delay(1000);

    initializeScreen("WFH","PTorrezao");
    delay(10000);

    initializeScreen("OOO","PTorrezao");
    delay(10000);

    initializeScreen("@OFFICE","PTorrezao");
    delay(10000);
    // display.drawExampleBitmap(gImage_quote, 0, 0, GxEPD_WIDTH,GxEPD_HEIGHT, GxEPD_BLACK);
    // display.update();
    //  Serial.println("??? asd...yeey :)");
    // delay(10000);
    // display.drawExampleBitmap(gImage_quote2, 0, 0, GxEPD_WIDTH,GxEPD_HEIGHT, GxEPD_BLACK);
    // display.update();
    // delay(10000);
    // display.drawExampleBitmap(gImage_quote3, 0, 0, GxEPD_WIDTH,GxEPD_HEIGHT, GxEPD_BLACK);
    // display.update();
    // delay(10000);
}
