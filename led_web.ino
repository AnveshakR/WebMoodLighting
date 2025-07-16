#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NeoPixelBus.h>
#include <WebServer.h>
#include "env.h"

WebServer server(80);
float bandLevels[10] = {0}; // FFT band data from audio visualization

NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(NUM_LEDS, LED_PIN);

int rgb[3]; // Current RGB color values
String current_display_type = ""; // Track current mode for interruption checking

// Update ESP32 IP address in Home Assistant
void updateIPInHA() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(HA_HOST) + "/api/states/" + HA_IP_ENTITY;
  String payload = "{\"state\": \"" + WiFi.localIP().toString() + "\"}";

  http.begin(url);
  http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
  http.addHeader("Content-Type", "application/json");

  int code = http.sendRequest("POST", payload);
  if (code != 200) {
    Serial.println("IP update failed: " + String(code));
  }
  http.end();
}

// Get state from Home Assistant entity
bool getStateFromHA(JsonDocument& doc, String state_entity) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = String(HA_HOST) + "/api/states/" + state_entity;

  http.begin(url);
  http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));

  int code = http.GET();

  if (code == 200) {
    String payload = http.getString();
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
      Serial.println("JSON parse error: " + String(err.c_str()));
      return false;
    }
    http.end();
    return true;
  } else {
    Serial.println("HA request failed: " + String(code));
    http.end();
    return false;
  }
}

// Check if display type has changed (for interrupting long-running animations)
bool shouldContinueMode(String expected_mode) {
  StaticJsonDocument<512> check_doc;
  if (getStateFromHA(check_doc, HA_LED_STATE_ENTITY)) {
    auto state_str = check_doc["state"].as<const char*>();
    StaticJsonDocument<200> parsed_state;
    DeserializationError err = deserializeJson(parsed_state, state_str);
    if (!err) {
      String current_mode = parsed_state["display_type"];
      return (current_mode == expected_mode);
    }
  }
  return false; // If we can't check, assume we should stop
}

// Handle FFT data updates from audio visualization
void handleUpdateFFT() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }

  String body = server.arg("plain");
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    Serial.println("FFT JSON error: " + String(error.c_str()));
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  JsonArray arr = doc["band_levels"];
  if (!arr || arr.size() != 10) {
    server.send(400, "text/plain", "Expected 10 values");
    return;
  }

  // Update band levels array
  for (int i = 0; i < 10; i++) {
    bandLevels[i] = arr[i].as<float>();
  }

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}



// JSON status endpoint
void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["status"] = "ok";
  doc["ip"] = WiFi.localIP().toString();
  doc["heap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  doc["wifi_rssi"] = WiFi.RSSI();
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}


void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 starting...");
  
  // Initialize LED strip
  strip.Begin();
  strip.Show(); // Clear all LEDs
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 30) {
    delay(1000);
    wifi_attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  } else {
    Serial.println("WiFi connection failed");
    return;
  }

  // Setup web server endpoints
  server.on("/status", handleStatus);
  server.on("/update_fft", HTTP_POST, handleUpdateFFT);
  
  server.begin();
  Serial.println("Server started");
  
  // Update IP in Home Assistant
  updateIPInHA();
  Serial.println("Setup complete");
}

void loop() {
  // Handle incoming web requests
  server.handleClient();
  
  // Get current LED state from Home Assistant
  StaticJsonDocument<512> state_doc;

  if (getStateFromHA(state_doc, HA_LED_STATE_ENTITY)) {
    auto state_str = state_doc["state"].as<const char*>();
    StaticJsonDocument<200> final_state_doc;
    DeserializationError err1 = deserializeJson(final_state_doc, state_str);
    
    if (err1) {
      Serial.println("State parse error: " + String(err1.c_str()));
    } else {
      // Update current display type and color
      current_display_type = final_state_doc["display_type"].as<String>();
      hexToRGB(final_state_doc["color_hex"], rgb);

      // Execute appropriate LED mode
      if (current_display_type == "solid") {
        solid_mode();
      }
      else if (current_display_type == "breathing") {
        breathing_mode();
      }
      else if (current_display_type == "AV") {
        AV_mode("2");
      }
      else if (current_display_type == "spectrum") {
        spectrum_mode(20);
      }
    }
  }

  delay(1);
}