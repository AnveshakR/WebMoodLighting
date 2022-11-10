#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <EEPROM.h>
#include <WiFiUdp.h>
#include <NeoPixelBus.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "arduinoFFT.h"
//#include <ArduinoOTA.h>
#include "wifi.h"
#include "webpage.h"


#define EEPROM_SIZE 4

// OLED Screen initialization
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SCL D3 SDA D4 ESP 12E
// SCL D1 SDA D2 ESP 12F

#define NUM_LEDS 300 // Number of LEDs in strip

// FFT settings values have to be declared after creating an FFT object
arduinoFFT FFT = arduinoFFT();
#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000
#define amplitude 200
#define sound A0

// FFT variables
unsigned int sampling_period_us;
unsigned long microseconds;
unsigned long oldTime, newTime;
double vReal[SAMPLES];
double vImag[SAMPLES];
int led_bands[8];
float scale;

// Neopixelbus function
NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(NUM_LEDS);
//LED DATA ON RX PIN

// gauss func for breathing
int gauss_size = 100;
float gauss[100] = { 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 8, 9, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 26, 28, 31, 34, 37, 40, 43, 47, 45, 54, 57, 60, 64, 67, 70, 73, 76,
                     78, 80, 82, 83, 84, 85, 85, 85, 84, 83, 82, 80, 78, 76, 73, 70, 67, 64, 60, 57, 54, 45, 47, 43, 40, 37, 34, 31, 28, 26, 23, 21, 19, 17, 16, 14, 13, 12, 11, 10,
                     9, 9, 8, 7, 7, 7, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5 };

AsyncWebServer server(80);

// webserver variables
String hex_val, picker_val, color_val;
int breath_val = 45, r_val, g_val, b_val, display_radio_val=69;
float brightness, newval, oldval, slope, scaleval = 0.5;
const char* ssid = WLAN;
const char* password = PASS;
const char* picker_param = "picker_input";
const char* display_radio = "display_input";
const char* breath_param = "breath_input";
bool val_change = true;

bool ap_status = false;
String ip, current_wifi;

#include "ledfunc.h"

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void display_text(String text, int x, int y, int fontsize) // displays text at (x,y) coordinate with specified fontsize
{ 
  display.setTextSize(fontsize);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(x, y);
  display.println(text);
  display.display();
  delay(5);
}

void display_rect(int x, int y, int w, int h) // makes clear rectangle at (x,y) of (w,h) width and height
{ 
  display.fillRect(x, y, w, h, BLACK);
  display.display();
  delay(5);
}

void color_set() // sets global color variables
{
  long number = strtol(&color_val[1], NULL, 16);
  r_val = String(number >> 16).toInt();
  g_val = String(number >> 8 & 0xFF).toInt();
  b_val = String(number & 0xFF).toInt();


  //  EEPROM.write(0, r_val);
  //  EEPROM.write(1, g_val);
  //  EEPROM.write(2, b_val);
  //  EEPROM.commit();
}

void setup() {

  //  EEPROM.begin(EEPROM_SIZE);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  display_text("WiFi LED", 14, 0, 2);

  Serial.begin(115200);
  
  strip.Begin();
  strip.Show();

  pinMode(sound, INPUT);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); // setting sampling frequency

  //AsyncWebServer server(80);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  current_wifi = ssid;

  if (WiFi.waitForConnectResult() != WL_CONNECTED) // waits until connection is unavailable
  {
    display_text("Unable to connect to", 0, 30, 1);
    display_text(ssid, 0, 40, 1);
    display_text("Switching to AP Mode.", 0, 45, 1);
    ap_status = true; // switch to access point mode if wifi not available
    current_wifi = "WiFi_LED";
    delay(4500);
  }
  
  if (ap_status == true)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    Serial.println(WiFi.softAP("WiFi_LED", "1234") ? "Ready" : "Failed!"); // AP wifi name and password
  }
  display_rect(0, 20, 128, 46);

  // ArduinoOTA.onStart([]() {
  //   Serial.println("Start");
  // });
  // ArduinoOTA.onEnd([]() {
  //   Serial.println("\nEnd");
  // });
  // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  // });
  // ArduinoOTA.onError([](ota_error_t error) {
  //   Serial.printf("Error[%u]: ", error);
  //   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  //   else if (error == OTA_BEGIN_ERROR)
  //     Serial.println("Begin Failed");
  //   else if (error == OTA_CONNECT_ERROR)
  //     Serial.println("Connect Failed");
  //   else if (error == OTA_RECEIVE_ERROR)
  //     Serial.println("Receive Failed");
  //   else if (error == OTA_END_ERROR)
  //     Serial.println("End Failed");
  // });
  // ArduinoOTA.begin();

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(ap_status? WiFi.softAPIP().toString() : WiFi.localIP().toString());

  display_text("IP:", 0, 20, 1);
  display_text(ap_status? WiFi.softAPIP().toString() : WiFi.localIP().toString(), 17, 20, 1); // display IP on screen

  display_text("Wi-Fi: ", 0, 32, 1);
  display_text(current_wifi, 45, 32, 1); // display wifi name on screen

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) { // launches webpage and listens to requests
    request->send_P(200, "text/html", index_html);
    String input_parameter;

    if (request->hasParam("picker_input")) // checks if color picker request is there (always will be)
    {
      color_val = request->getParam("picker_input")->value();
      if (color_val != "#010101") // #010101 is default value, doesnt show on strip
      {
        color_set();
        val_change = true; // activate flag
      }
    }

    if (request->hasParam("display_input")) 
    {
      display_radio_val = (request->getParam("display_input")->value()).toInt();
      val_change = true; // activate flag
    }

  });
  server.onNotFound(notFound);
  server.begin();

  //  r_val = EEPROM.read(0);
  //  g_val = EEPROM.read(1);
  //  b_val = EEPROM.read(2);
  //  display_radio_val = EEPROM.read(3);
}

void loop() {

  if (val_change == true) { // if flag is active

    display_text("Mode: ", 0, 42, 1);
    display_rect(45, 42, 86, 10);
    switch (display_radio_val) // display mode on screen
    {
      case 0: display_text("Solid", 45, 42, 1); break;
      case 1: display_text("Breathing", 45, 42, 1); break;
      case 2: display_text("Audio-Viz", 45, 42, 1); break;
      case 3: display_text("Spectrum", 45, 42, 1); break;
      default: display_text("None", 45, 42, 1); break;
    }

    display_text("Color: ", 0, 52, 1);
    display_rect(45, 52, 86, 10);
    display_text(String(r_val)+','+String(g_val)+','+String(b_val), 45, 52, 1); //display RGB values on screen
  }

  if (display_radio_val == 0)  //solid mode
  {
    solid_mode();
  } 
  
  else if (display_radio_val == 1)  //breathing mode
  {
    breathing_mode();
  }

  else if (display_radio_val == 2)   //AV mode
  {
    basicAV_mode();
  }

  else if (display_radio_val == 3)  //Spectrum mode
  {
    spectrum_mode(20);
  }
}
