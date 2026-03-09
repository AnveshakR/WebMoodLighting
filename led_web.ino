#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NeoPixelBus.h>
#include <WebServer.h>
#include "env.h"
#include "ledfunc.h"

// Server
WebServer server(80);

// Strip
NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip(NUM_LEDS, LED_PIN);
int rgb[3];

// State
float bandLevels[10] = {0};
String current_display_type = "";

// Entities — derived from ESP_NAME in setup()
String ha_light_entity;
String ha_display_entity;
String ha_ip_webhook;


/* Posts the ESP's current IP to Home Assistant via webhook. */
void updateIPInHA() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String payload = "{\"ip\": \"" + WiFi.localIP().toString() + "\"}";

  http.begin(ha_ip_webhook);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(payload);
  if (code != 200 && code != 201) {
    Serial.println("IP webhook failed: " + String(code));
  }
  http.end();
}

/* Fetches a HA entity state and deserializes it into doc. Returns false on any failure. */
bool getStateFromHA(JsonDocument& doc, const String& entity_id) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.begin(String(HA_HOST) + "/api/states/" + entity_id);
  http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));

  int code = http.GET();

  if (code == 200) {
    DeserializationError err = deserializeJson(doc, http.getString());
    http.end();
    if (err) {
      Serial.println("JSON parse error: " + String(err.c_str()));
      return false;
    }
    return true;
  }

  Serial.println("HA request failed (" + entity_id + "): " + String(code));
  http.end();
  return false;
}

/* Reads light.<name> from HA. Sets is_on and updates rgb[]. Returns false on failure. */
bool getLightState(bool& is_on) {
  StaticJsonDocument<512> doc;
  if (!getStateFromHA(doc, ha_light_entity)) return false;

  is_on = (String(doc["state"].as<const char*>()) == "on");

  if (is_on) {
    JsonArray rgb_arr = doc["attributes"]["rgb_color"];
    if (rgb_arr && rgb_arr.size() == 3) {
      rgb[0] = rgb_arr[0];
      rgb[1] = rgb_arr[1];
      rgb[2] = rgb_arr[2];
    } else {
      rgb[0] = rgb[1] = rgb[2] = 255; // IMPORTANT: default white if HA has no color set yet
    }
  }

  return true;
}

/* Reads select.<name>_display_state from HA. Returns current_display_type unchanged on failure. */
String getDisplayMode() {
  StaticJsonDocument<256> doc;
  if (!getStateFromHA(doc, ha_display_entity)) return current_display_type;
  return doc["state"].as<String>();
}

/* Returns true if the current HA display mode still matches expected_mode. Used by animations. */
bool shouldContinueMode(String expected_mode) {
  StaticJsonDocument<256> doc;
  if (!getStateFromHA(doc, ha_display_entity)) return false;
  return doc["state"].as<String>() == expected_mode;
}

/* Sets all LEDs to black. */
void clearStrip() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.SetPixelColor(i, RgbColor(0, 0, 0));
  }
  strip.Show();
}

/* POST /update_fft — receives 10 FFT band level floats and updates bandLevels[]. */
void handleUpdateFFT() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  JsonArray arr = doc["band_levels"];
  if (!arr || arr.size() != 10) {
    server.send(400, "text/plain", "Expected 10 values");
    return;
  }

  for (int i = 0; i < 10; i++) {
    bandLevels[i] = arr[i].as<float>();
  }

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

/* GET /status — returns basic ESP health info as JSON. */
void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["status"]    = "ok";
  doc["ip"]        = WiFi.localIP().toString();
  doc["heap"]      = ESP.getFreeHeap();
  doc["uptime"]    = millis() / 1000;
  doc["wifi_rssi"] = WiFi.RSSI();

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}


void setup() {
  Serial.begin(115200);

  ha_light_entity   = String("light.")   + ESP_NAME;
  ha_display_entity = String("select.")  + ESP_NAME + "_display_state";
  ha_ip_webhook     = String(HA_HOST)    + "/api/webhook/led_esp_" + ESP_NAME + "_ip";

  strip.Begin();
  strip.Show();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 30) {
    delay(1000);
    wifi_attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed");
    return;
  }
  Serial.println("WiFi connected: " + WiFi.localIP().toString());

  server.on("/status",     handleStatus);
  server.on("/update_fft", HTTP_POST, handleUpdateFFT);
  server.begin();

  updateIPInHA();
}

void loop() {
  server.handleClient();

  bool light_on;
  if (!getLightState(light_on)) {
    delay(1000);
    return;
  }

  if (!light_on) {
    clearStrip();
    delay(100);
    return;
  }

  current_display_type = getDisplayMode();

  if      (current_display_type == "solid")     solid_mode();
  else if (current_display_type == "breathing") breathing_mode();
  else if (current_display_type == "AV")        AV_mode("2");
  else if (current_display_type == "spectrum")  spectrum_mode(20);

  delay(1);
}
