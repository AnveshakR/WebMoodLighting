# LED_WEB

A smart LED lighting control system that integrates WS2812B RGB LED strips with Home Assistant for flexible, remote-free control.

## Overview

This project controls a WS2812B RGB LED strip connected to an ESP32 by interfacing with a **Home Assistant** server on the local network. A custom HA integration creates per-ESP entities and a Lovelace dashboard widget for each strip, with no intermediary server required.

### Key Features

- **No Remote Required**: Control lights directly through Home Assistant
- **Easy Scaling**: Adding a new ESP requires only flashing the firmware with a new name and adding one integration entry in HA — no other changes needed
- **Dashboard Widget**: Each ESP gets a native HA color picker and display mode dropdown
- **Real-Time Audio Visualization**: Strip brightness can follow live audio FFT data posted to the ESP's local endpoint

### Available Lighting Modes

- Solid color display
- Breathing color display
- Audio-visualizer color display (requires external audio source posting to `/update_fft`)
- Color spectrum

## Installation

### 1. Home Assistant Integration

Clone the repo and copy the integration folder into your HA config directory from within the HA Terminal addon:

```bash
git clone https://github.com/AnveshakR/WebMoodLighting.git /tmp/led_web
cp -r /tmp/led_web/custom_components/led_esp /config/custom_components/
rm -rf /tmp/led_web
```

After copying, restart HA:

```bash
ha core restart
```

### 2. Add an ESP in Home Assistant

After restart, go to **Settings → Devices & Services → Add Integration** and search for **LED ESP**. Enter a name for the ESP (e.g. `desk`). This creates three entities:

| Entity | ID | Purpose |
|--------|-----|---------|
| Light | `light.<name>` | RGB color and on/off |
| Select | `select.<name>_display_state` | Display mode dropdown |
| Sensor | `sensor.<name>_ip` | ESP IP address (updated on boot) |

### 3. Flash the ESP32

Configure `env.h` with the same name used in step 2, then flash `led_web.ino` via the Arduino IDE. The ESP must be flashed and started after the integration entry exists in HA, since it posts its IP to HA on boot.

### 4. Lovelace Dashboard

Add a **Light card** pointing at `light.<name>` for the color picker, and an **Entities card** or **Tile card** with `select.<name>_display_state` below it. Suggested YAML for a single ESP named `test_lights`:

```yaml
- type: entities
  title: Test Lights LED Strip
  entities:
    - entity: light.test_lights
      name: Color & Power
    - entity: select.test_lights_display_state
      name: Display Mode
    - entity: sensor.test_lights_ip
      name: IP
      icon: mdi:ip-network
```

## Configuration

### env.h (ESP32 Arduino Sketch)

```cpp
#ifndef ENV_H
#define ENV_H

// WiFi
const char* WIFI_SSID     = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";

// Home Assistant
const char* HA_HOST  = "http://YOUR_HA_IP:8123";
const char* HA_TOKEN = "your_long_lived_access_token"; // Profile → Long-Lived Access Tokens

// Identity — IMPORTANT: must match the name entered in the LED ESP HA integration
const char* ESP_NAME = "your_esp_name";

// Strip
#define LED_PIN  5      // GPIO pin connected to the LED strip data line
#define NUM_LEDS 300    // total number of LEDs in your strip

#endif
```

## 3D Printed Enclosure

![3D Printed Box](images/box.png)

The [3D print files](3D%20print%20files/) folder contains STL files for a custom enclosure to house the ESP32 and wire connections. View them interactively on GitHub: [Box](3D%20print%20files/esp%20box.STL) · [Lid](3D%20print%20files/esp%20box%20lid.STL)

### Required Components

1. **USB-C Connector**: [USB-C Panel Mount Cable](https://a.co/d/ibNtGKY)
2. **WAGO Lever Nut Connectors**: [3-conductor lever nuts × 3](https://a.co/d/3chznWW)
3. **ESP32 Board**: [ESP32 DOIT DevKit V1](https://a.co/d/75TbzBZ)
4. **LED Strip**: [WS2812B LED Strip](https://a.co/d/dlXuyIC)
5. **M3×8 Screws**: 4× screws to secure the ESP32 to the box

### Wiring Connections

Use the three WAGO lever nut connectors to make the following connections:

**Connector 1 (Power +):**
- VIN pin of ESP32
- Positive (+) wire of USB-C connector
- Positive (+) wire of LED strip

**Connector 2 (Ground -):**
- GND pin of ESP32
- Negative (-) wire of USB-C connector
- Negative (-) wire of LED strip

**Connector 3 (Data Signal):**
- Data/TRIG pin of LED strip
- GPIO pin of ESP32 (as defined by `LED_PIN` in env.h, recommended: GPIO 5)
