#include <FastLED.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>


// Enter your WiFi SSID and password
char ssid[] = "AccessPoint";          // your network SSID (name)
char pass[] = "password";    // your network password (use for WPA, or use as key for WEP)
String hostname = "Skimap";
int keyIndex = 0;                      // your network key Index number (needed only for WEP)

//int status = WL_IDLE_STATUS;

#define SERVER "api.fnugg.no"
#define PATH   "/get/resort/2"

#define DOUT_PIN 16
#define NUM_LEDS 7
#define NUM_LIFTS 32
CRGB leds[NUM_LEDS];

const char lift_names[NUM_LIFTS][32] = { \
  "T3 Hygglo", "T6 Eventyr", "T4 Fryvil", "T5 Tussi", "T2 Fjellekspressen", "T1 Liekspressen", "S7 Håvitrekket", "S6 Oletrekket", \
  "T7 Sindretrekket", "T8 Knetta", "T9 Setertrekket", "T10 Hesten", "T11 Eventyr 2", \
  "S1 Skihytta Ekspress", "S4 Tolver'n", "S3 Valleheisen", "H1 Høgekspressen", "H2 Høgegga", "H3 Svart'n", "F1 Brynbekken", "F2 Toppekspressen", "F3 Kanken", \
  "F5 Skarven", "F6 Hytteheis 1", "F7 Stormyra 2", "F8 Stormyra", "F9 Stjerna", "F10 Smotten", "F11 Isiz", "F12 Familietrekket", "F13 Myrsnipa", "T3 Hygglo 2"};

void setup() {
  //Initialize serial and wait for port to open:
  FastLED.addLeds<WS2812, DOUT_PIN, GRB>(leds, NUM_LEDS);   
  delay(1000);
  Serial.begin(115200);
  leds[0] = CRGB::White; FastLED.show();
  delay(1000);
  WiFi.setHostname(hostname.c_str());
  // Connect to WPA/WPA2 network
  WiFi.begin(ssid, pass);
  delay(1000);
  //while (!Serial) {
    // wait for serial port to connect. 
    delay(10); 
  //}
  
  delay(1000);
  leds[1] = CRGB::Yellow; FastLED.show();

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

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

  printWifiStatus();
}

uint32_t bytes = 0;

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

  // wait until we get a newline
  client.find("\r\n\r\n", 4);
  while (client.peek() != '{' != 0) {
    client.read();
  }

  leds[3] = CRGB::Yellow; FastLED.show();
  
  Serial.print("Buffer size: ");
  Serial.println(buffer_size);
  Serial.println("---");
 // Serial.println(client.readString());
  Serial.println("---");

  DynamicJsonDocument doc(32768);  // Allocate JSON document

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
      if (strcmp(lift_name, lift_names[i]) == 0) {
        Serial.print(lift_name);
        Serial.print(" : ");
        if (i < NUM_LEDS) {
          if (strcmp(lift_status, "1") == 0) {
            leds[i] = CRGB::Green;
            Serial.println("Open");     
          } else {
            leds[i] = CRGB::Red;
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
/*
  int j = 0;
  while(1){
    leds[j%NUM_LEDS] = CRGB::White; FastLED.show();
    delay(30);
    leds[j++%NUM_LEDS] = CRGB::Black; FastLED.show();
  }
*/
  delay(100 * 1000);
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


