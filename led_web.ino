#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DATA D5
#define NUM_LEDS 100 

float gauss[100] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 9, 10, 12, 14, 16, 18, 20, 23, 26, 29, 32, 36, 40, 44, 48, 52, 56, 61, 65, 69, 73, 77, 81, 85, 88, 91, 94, 96, 98, 99, 100, 100, 100, 99, 98, 96, 94, 91, 88, 85, 81, 77, 73, 69, 65, 61, 56, 52, 48, 44, 40, 36, 32, 29, 26, 23, 20, 18, 16, 14, 12, 10, 9, 7, 6, 5, 
4, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0};

AsyncWebServer server(80);

String hex_val,picker_val,display_radio_val,color_radio_val;
int breath_val=50,r_val,g_val,b_val;
float brightness;
float gmma = 0.14; // affects the width of peak (more or less darkness)
float beta = 0.5; // shifts the gaussian to be symmetric
const char* ssid = "Trojan";
const char* password = "NModi1@9";
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

//String num_to_text(int num)
//{
//  String num = String(num);
//  for(int i=3;i>num.length();i++)
//  {
//    num = "0"+num;
//  }
//}

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
      FastLED.show();
      FastLED.delay(1);
    }
  }
  else if (display_radio_val=="breathing")
  {
    //Serial.println("dimming");
    for (int i=100; i>=0; i--)
    {
      Serial.println(gauss[i]);
      for (int c=0; c<NUM_LEDS; c++)
      {
        leds[c] = CRGB(r_val,g_val,b_val);
        FastLED.setBrightness(gauss[i]);     
        FastLED.show();
        //FastLED.delay(5);
      }
      FastLED.delay(1);
    }
  }
}
