# LED_WEB

This project is used to control a `WS2812B` RGB LED strip using a webpage local-hosted on Wi-Fi by an ESP8266 (`12-E` chip). 

The motivation behind this project was so that there would be no need for yet another remote to control the lights as is normally the case. Also, having a webpage allows for easy updates to the software to update and add new functions for the same hardware.

The design has the following components:
- `ESP8266 (12-E)`
- `SSD1306 128x64 OLED Display (SPI config)`
- `MAX4466 Electret Microphone Amplifier with Adjustable Gain Module`
- `10kΩ Potentiometer`
- `1μF 63V Capacitor`

The current program has the following functions:
- Solid color display
- Breathing color display
- Basic Audio-Visualizer color display

The audio visualizer is based on an FFT transform of the input data from the MAX4466.

I have included the Arduino program, along with the KiCAD project for the PCB design.

---
> Circuit Schematic
>
> ![Schematic](/images/led_web_schematic.png)

---
| PCB front view | PCB Back View |
| --- | --- |
|![PCB Front](/images/pcb_front.png)|![PCB Back](/images/pcb_back.png)|

### TO-DO

- [ ] Proper commenting lol
- [ ] Select particular audio freq for AV
- [ ] Select Wi-Fi network
- [ ] Load last config from memory
- [ ] Over-voltage protection for components
- [ ] Convert THT into SMD (improbable tbh)