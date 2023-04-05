///////////////////////////////////////////////
//                                           //
// Lift map                                  //
//                                           //
// Fetches lift open status from fnugg.no    //
// and displays it on addressable LEDs.      //
//                                           //
///////////////////////////////////////////////

#include <Arduino.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include "cfg_trysil.h"

#define SERVER "api.fnugg.no"
#define PATH   "/get/resort/" RESORT_ID
#define HOST_NAME "Lift map " RESORT_NAME

#define NUM_LEDS 31
#define DOUT_PIN 16

CRGB leds[NUM_LEDS];

void setup() {
  //Initialize serial and wait for port to open:
  FastLED.addLeds<WS2812, DOUT_PIN, GRB>(leds, NUM_LEDS);   
  delay(1000);
  Serial.begin(115200);
  leds[0] = CRGB::White; FastLED.show();
  delay(1000);
  WiFi.setHostname(HOST_NAME);
  // Connect to WPA/WPA2 network
  WiFi.begin(wifi_ssid, wifi_password);
  delay(1000);

  leds[1] = CRGB::Yellow; FastLED.show();

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(wifi_ssid);

  while (WiFi.status() != WL_CONNECTED) {
    leds[2] = CRGB::White; FastLED.show();
    delay(250);
    Serial.print(".");
    leds[2] = CRGB::Black; FastLED.show(); delay(50);
    delay(250);
  }
  leds[2] = CRGB::Yellow; FastLED.show();

  Serial.println("");
  Serial.println("Connected to WiFi");

}

void loop() {
  WiFiClientSecure client;
  client.setInsecure(); // don't use a root cert

  Serial.println("\nStarting connection to server...");

  // if you get a connection, report back via serial:
  if (client.connect(SERVER, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET " PATH " HTTP/1.1");
    client.println("Host: " SERVER);
    client.println("Connection: close");
    client.println();
  }

  // Check HTTP status

  char status[32] = {0};
  int buffer_size = client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Get rid of responses prior to first '{'
  client.find("\r\n\r\n", 4);
  while (client.peek() != '{') {
    client.read();
  }

  leds[3] = CRGB::Yellow; FastLED.show();
  
  Serial.print("Buffer size: ");
  Serial.println(buffer_size);

  // Allocate JSON document with space for whole server response
  DynamicJsonDocument doc(32768);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    leds[4] = CRGB::Blue; FastLED.show();
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  } else {
    leds[4] = CRGB::Yellow; FastLED.show();
  }
  
  // Extract values
  JsonObject source_lifts = doc["_source"]["lifts"];


  for (JsonObject lift : source_lifts["list"].as<JsonArray>()) {
    const char* lift_name   = lift["name"]; // "T1 Liekspressen", "T2 ...
    const char* lift_status = lift["status"]; // "1", "1", "1", "1", ...

    for (int i = 0; i < NUM_LIFTS; i++) {
      if (strcmp(lift["name"], lift_names[i]) == 0) {
        Serial.print(lift_name);
        Serial.print(" : ");
        if (i < NUM_LEDS) {
          if (strcmp(lift_status, "1") == 0) {
            leds[i].setRGB(0,1,0);
            Serial.println("Open");     
          } else {
            leds[i].setRGB(1,0,0);
            Serial.println("Closed"); 
          }
          FastLED.show();
        }
      }
    }
  }

    // Disconnect
  Serial.println("disconnecting from server.");

  client.stop();
  delay(1000);

  delay(100 * 1000);
}