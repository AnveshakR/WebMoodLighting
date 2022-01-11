#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <ESP8266mDNS.H>
#include <WiFiUdp.h>
#include <FastLED.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>
#include "wifi.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SCL D3 FOR ESP 12E
// SDA D4 FOR ESP 12E
#define DATA D5
#define NUM_LEDS 100 

int gauss_size = 100;
float gauss[100] = {5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 8, 9, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 26, 28, 31, 34, 37, 40, 43, 47, 50, 54, 57, 60, 64, 67, 70, 73, 76, 
78, 80, 82, 83, 84, 85, 85, 85, 84, 83, 82, 80, 78, 76, 73, 70, 67, 64, 60, 57, 54, 50, 47, 43, 40, 37, 34, 31, 28, 26, 23, 21, 19, 17, 16, 14, 13, 12, 11, 10, 
9, 9, 8, 7, 7, 7, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5};

AsyncWebServer server(80);

String hex_val,picker_val,display_radio_val,color_radio_val;
int breath_val=50,r_val,g_val,b_val;
float brightness;
const char* ssid = WLAN;
const char* password = PASS;
const char* hex_param = "hex_input";
const char* picker_param = "picker_input";
const char* color_radio = "color_input";
const char* display_radio = "display_input";
const char* breath_param = "breath_input";
bool val_change = false;

CRGB leds[NUM_LEDS];

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>LED Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html {
        font-family: Sans-Serif;
        display: block;
        background-color: #222831;
        color: #EEEEEE;
        text-align: center
      }

      p {
        color: #EEEEEE;
      }

      h2 {
        font-size: 3.0rem;
        color: #7BC74D
      }

      input {
        background-color: #393E46;
        color: #EEEEEE
      }

      button {
        background-color: #393E46;
        color: #EEEEEE
      }

      form {
        padding: 15px
      }

      ;
    </style>
  </head>
  <body>
    <h2>LED Control</h2>
    <form action="/">
      <p>Select color:</p>
      <input type="radio" id="hex" name="color_input" value="hex">Hex value: # <input type="text" name="hex_input" id="hex" />
      <br>
      <br>
      <input type="radio" id="picker" name="color_input" value="picker">Pick a color: <input type="color" name="picker_input" id="picker" />
      <br>
      <br>
      <p>Select display type:</p>
      <input type="radio" id="solid" name="display_input" value="solid">Solid Color <br>
      <br>
      <input type="radio" id="breathing" name="display_input" value="breathing">Breathing <br>
      <br>
      <br>
      <input type="submit" value="Submit">
    </form>
  </body>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void display_text(String text, int x, int y, int fontsize) 
{
  display.setTextSize(fontsize);             
  display.setTextColor(WHITE, BLACK);      
  display.setCursor(x,y);             
  display.println(text);
  display.display();
  delay(5);
}

void display_rect(int x, int y, int w, int h)
{
  display.fillRect(x,y,w,h,BLACK);
  display.display();
  delay(5);
}

void setup() {
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  display_text("WiFi LED",14,0,2);
  
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, DATA>(leds, NUM_LEDS);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting...");
    return;
  }

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  String ip = WiFi.localIP().toString();
  display_text("IP:", 0,20,1);
  display_text(ip, 17,20,1);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
    String input_parameter;
    if (request->hasParam("color_input") and request->hasParam("display_input")) {
      val_change = true;
      color_radio_val = request->getParam(color_radio)->value();
      display_radio_val = request->getParam(display_radio)->value();
      color_set(request);
    }

    else {}
  });
  server.onNotFound(notFound);
  server.begin();
}

void color_set(AsyncWebServerRequest *request)
{
  
  if(color_radio_val=="hex")
  {
    hex_val = request->getParam(hex_param)->value();
    long number = strtol(&hex_val[0], NULL, 16);
    r_val = String(number >> 16).toInt();
    g_val = String(number >> 8 & 0xFF).toInt();
    b_val = String(number & 0xFF).toInt();
  }
  else if(color_radio_val=="picker")
  {
    picker_val = request->getParam(picker_param)->value();
    long number = strtol( &picker_val[1], NULL, 16);     
    r_val = String(number >> 16).toInt();
    g_val = String(number >> 8 & 0xFF).toInt();
    b_val = String(number & 0xFF).toInt();
  }
}

void loop()
{ 
  if (val_change == true)
  {
    display_text("Red: ",0,32,1);
    display_rect(50,32,21,10);
    display_text(String(r_val), 50,32,1);
    
    display_text("Green: ",0,42,1);
    display_rect(50,42,21,10);
    display_text(String(g_val), 50,42,1);
    
    display_text("Blue: ",0,52,1);
    display_rect(50,52,21,10);
    display_text(String(b_val), 50,52,1);

    val_change = false;
  }

    
  if (display_radio_val=="solid")
  {
    for (int cur = 0; cur < NUM_LEDS; cur++) 
    {
      leds[cur] = CRGB(r_val,g_val,b_val);
      FastLED.setBrightness(85);
    }
    FastLED.show();
    FastLED.delay(1);
  }
  else if (display_radio_val=="breathing")
  {
    for (int i=gauss_size-1; i>=0; i--)
    {
      Serial.println(gauss[i]);
      for (int c=0; c<NUM_LEDS; c++)
      {
        leds[c] = CRGB((r_val*gauss[i])/100,(g_val*gauss[i])/100,(b_val*gauss[i])/100);
      }
      FastLED.show();
      FastLED.delay(32);
    }
  }
}
