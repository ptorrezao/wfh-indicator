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
#include <GxGDEM029T94/GxGDEM029T94.cpp> // 2.9" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include "BitmapGraphics.h"
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <string>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

GxIO_Class io(SPI, SS, 17, 16);
GxEPD_Class display(io, 16, 4);

const char *name = "FreeMono12pt7b";
const GFXfont *f = &FreeMono12pt7b;
const GFXfont *f9 = &FreeMono9pt7b;
const GFXfont *f9b = &FreeMonoBold9pt7b;
const GFXfont *f24b = &FreeMonoBold24pt7b;

static String currentUser = "";
static String currentState = "";

static String tempCurrentUser = "";
static String tempCurrentState = "";

#define MQTT_HOST "hm-wrl-srv01"
#define MQTT_PORT 1883
#define MQTT_USER "esp32"
#define MQTT_PWD "esp32"

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
/*
    Change the Configurations on the file configs.cpp
*/

void SetText(const String bold, const String text)
{
  display.setFont(f9b);
  display.print(bold);
  display.setFont(f9);
  display.println(text);
}

void turnLedToColor(uint32_t color)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
  FastLED.show();
}

void cleanScreen()
{
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

void initializeScreen(const String state, const String user)
{
  if (currentUser != user || currentState != state)
  {
    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);
    display.update();
    display.setCursor(0, 15);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 15);
    SetText("User:", user);
    display.setCursor(80, 15);
    display.setFont(f24b);
    display.println();
    display.println(" " + state);
    display.update();
    delay(1000);
    currentUser = user;
    currentState = state;
  }
}

void initializeNetwork()
{
  Serial.begin(BAUDRATE);
  WiFiManager wifiManager;
  wifiManager.autoConnect(APModeLabel);
  Serial.println("Device Connected to the Network.");

  display.setFont(f9);
  display.fillScreen(GxEPD_WHITE);
  display.update();
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(0, 15);

  SetText("SSID:", wifiManager.getWiFiSSID());
  SetText("IP:", WiFi.localIP().toString());
  SetText("Mac:", WiFi.macAddress());

  turnLedToColor(CRGB::White);
  FastLED.setBrightness(100);
  delay(500);
  FastLED.setBrightness(BRIGHTNESS);
  turnLedToColor(CRGB::Black);
}

void initializeLed()
{
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  for (int i = 0; i < NUM_LEDS; i++)
  {
    int previous = NUM_LEDS - 1;
    if (i >= 1)
    {
      previous = i - 1;
    }

    leds[previous] = CRGB::Black;
    leds[i] = CRGB::White;
    delay(20);
    FastLED.show();
  }

  turnLedToColor(CRGB::Black);
}

void connectToMqtt()
{
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent)
{
  Serial.println("Connected to MQTT.");
  uint16_t packetIdSub = mqttClient.subscribe("presenceindicator/ptorrezao", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected())
  {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  Serial.println("Publish received.");
  Serial.print("  Topic: ");
  Serial.println(topic);
  Serial.print("  Payload: ");
  Serial.println(payload);

  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload);

  tempCurrentUser = doc["Person"].as<String>();
  tempCurrentState = doc["State"].as<String>();
  turnLedToColor(atoi(doc["Color"].as<String>().c_str()));
}

void onMqttPublish(uint16_t packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    connectToMqtt();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WiFi lost connection");
    xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    break;
  }
}

void setup()
{
  cleanScreen();
  initializeLed();
  initializeNetwork();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setCredentials(MQTT_USER, MQTT_PWD);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToMqtt();
}

void loop()
{
  initializeScreen(tempCurrentState, tempCurrentUser);
  delay(100);
}
