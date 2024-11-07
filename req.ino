#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "AP184A";
const char* password = "marcia2518";
#define RELAY_PORT 2

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, LOW); 

  pinMode(RELAY_PORT, OUTPUT);
  digitalWrite(RELAY_PORT, LOW); 

  Serial.println();
  Serial.println();
  Serial.println();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
}

void loop() {

  // wait for WiFi connectionç
  if (WiFi.status() == WL_CONNECTED) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation
    client->setInsecure();
    
    // Create an HTTPClient instance
    HTTPClient https;
    
    // Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://switch-backend-2trd.onrender.com/api/v1/switch/arduino/123456789")) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // Start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been sent and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // File found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          
          // Allocate the JSON document
          // Use arduinojson.org/v6/assistant to compute the capacity.
          const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
          DynamicJsonDocument doc(capacity);
          
          // Parse JSON object
          DeserializationError error = deserializeJson(doc, payload);
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }
          
          // Extract values
          int status = doc["status"];
          bool data = doc["is_active"];

          Serial.println(data);
          
          // Check if data is true or false
          if (data) {
            Serial.println("Data is true.");
            digitalWrite(RELAY_PORT, LOW);
          } else {
            digitalWrite(RELAY_PORT, HIGH);
            Serial.println("Data is false.");
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  Serial.println();
}
