#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#define DATA D4
#define NUM_LEDS 20 

AsyncWebServer server(80);

const char* ssid = "sad wifi";
const char* password = "sadpassword";
String r_val,g_val,b_val,hex_val,picker_val;
const char* red_param = "red_input";
const char* green_param = "green_input";
const char* blue_param = "blue_input";
const char* hex_param = "hex_input";
const char* picker_param = "picker_input";

CRGB leds[NUM_LEDS];

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #0000FF;}
  </style>
  </head><body>
  <h2>LED Control</h2> 
  <p>Enter RGB values(0-255)</p>
  <form action="/">
    <label for="r">Red: </label> 
    <input type="number" name="red_input" id="red" step="1" min="0" max="255">
    
    <label for="g">Green: </label> 
    <input type="number" name="green_input" id="green" step="1" min="0" max="255">
    
    <label for="b">Blue: </label> 
    <input type="number" name="blue_input" id="blue" step="1" min="0" max="255">
    
    <button><input type="submit" value="Submit"></button>
  </form><br>

  <p>OR</p>
  
  <form action="/">
    <label for="hex">Enter hex value: #</label>
    <input type="text" name="hex_input" id="hex">
    <button><input type="submit" value="Submit"></button>
  </form><br>  
  
  <p>OR</p>
  
  <form action="/">
    <label for="picker">Select a color:</label>
    <input type="color" name="picker_input" id="picker">
    <button><input type="submit" value="Submit"></button>
  </form><br>
  
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, DATA>(leds, NUM_LEDS);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting...");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
    String input_parameter;
    if (request->hasParam(red_param) and request->hasParam(green_param) and request->hasParam(blue_param)) {
      r_val = request->getParam(red_param)->value();
      g_val = request->getParam(green_param)->value();
      b_val = request->getParam(blue_param)->value();
    }

    else if (request->hasParam(hex_param)){
      hex_val = request->getParam(hex_param)->value();
      long number = strtol(&hex_val[0], NULL, 16);     
      r_val = String(number >> 16);
      g_val = String(number >> 8 & 0xFF);
      b_val = String(number & 0xFF);
    }
    
    else if (request->hasParam(picker_param)){
      picker_val = request->getParam(picker_param)->value();
      long number = strtol( &picker_val[1], NULL, 16);     
      r_val = String(number >> 16);
      g_val = String(number >> 8 & 0xFF);
      b_val = String(number & 0xFF);
    }
    else {}
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop()
{ 
  byte r,g,b;
  for (int cur = 0; cur < NUM_LEDS; cur++) 
  {
    r = r_val.toInt();
    g = g_val.toInt();
    b = b_val.toInt();
    leds[cur] = CRGB(r,g,b);
    FastLED.show();
    FastLED.delay(10);
  }
}
