# LED_WEB

This project is used to control a `WS2812B` RGB LED strip connected to an ESP32 by interfacing it with a server on the local network running Home Assistant. Another server will be running a flask webserver with the ability to change the lights via a locally hosted webpage.   

The motivation behind this project was so that there would be no need for yet another remote to control the lights as is normally the case. Also, having a webpage allows for easy updates to the software to update and add new functions for the same hardware.

The second server pc also has a USB mic attached to it for access to real time audio data for accurate audio visualization on the lights,

The current program has the following functions:
- Solid color display
- Breathing color display
- Audio-Visualizer color display
- Color Spectrum
