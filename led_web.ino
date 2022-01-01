#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#define DATA D4
#define NUM_LEDS 20 
void color_set(String,String,AsyncWebServerRequest);

AsyncWebServer server(80);

String hex_val,picker_val,display_radio_val;
int breath_val=1000,r_val,g_val,b_val;
const char* ssid = "sad wifi";
const char* password = "sadpassword";
const char* hex_param = "hex_input";
const char* picker_param = "picker_input";
const char* color_radio = "color_input";
const char* display_radio = "display_input";
const char* breath_param = "breath_input";

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
        text-align: justify
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
      <input type="radio" id="breathing" name="display_input" value="breathing">Breathing: <input type="number" name="breath_input" id="breath" />ms 
      <br>
      <br>
      <br>
      <button><input type="submit" value="Submit"></button>
    </form>
  </body>
</html>)rawliteral";

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
    if (request->hasParam("color_input") and request->hasParam("display_input")) {
      String radio1 = request->getParam(color_radio)->value();
      display_radio_val = request->getParam(display_radio)->value();
      color_set(radio1, request);
    }

    else {}
  });
  server.onNotFound(notFound);
  server.begin();
}

void color_set(String color_radio_val, AsyncWebServerRequest *request)
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
  if (display_radio_val=="solid")
  {
    for (int cur = 0; cur < NUM_LEDS; cur++) 
    {
      leds[cur] = CRGB(r_val,g_val,b_val);
      FastLED.show();
      FastLED.delay(10);
    }
  }
  else if (display_radio_val=="breathing")
  {
//    r = r_val.toInt();
//    g = g_val.toInt();
//    b = b_val.toInt();
//    for (int c=0; c<NUM_LEDS; c++)
//      {
//        leds[c] = CRGB(r_val,g_val,b_val);
//        FastLED.show();
//        FastLED.delay(5);
//      }
    Serial.println("dimming");
    for (int i=255; i>0; i--)
    {
      Serial.println(i);
      for (int c=0; c<NUM_LEDS; c++)
      {
        leds[c] = CRGB(r_val,g_val,b_val);
        FastLED.setBrightness(i);     
        FastLED.show();
        //FastLED.delay(5);
      }
      FastLED.delay(round(breath_val/510));
    }
    Serial.println("lighting");
    for (int i=0; i<255; i++)
    {
     Serial.println(i);
     for (int c=0; c<NUM_LEDS; c++)
      {
        
        leds[c] = CRGB(r_val,g_val,b_val);
        FastLED.setBrightness(i);     
        FastLED.show();
        //FastLED.delay(5);
      }
      FastLED.delay(round(breath_val/510));
    } 
  }
}
