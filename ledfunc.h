#ifndef LEDFUNC_H
#define LEDFUNC_H

#include <NeoPixelBus.h>

// Externs
extern NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> strip;
extern int rgb[3];
extern float bandLevels[10];

// Prototype
bool shouldContinueMode(String expected_mode);


/* All LEDs set to the current rgb color at 85% brightness. */
void solid_mode() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.SetPixelColor(i, RgbColor(rgb[0]*0.85, rgb[1]*0.85, rgb[2]*0.85));
  }
  strip.Show();
  delay(1);
}

/* Pulses the strip through a Gaussian brightness curve. Checks HA for mode changes every frame. */
void breathing_mode() {
  int gauss_size = 100;
  float gauss[100] = { 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 8, 9, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 26, 28, 31, 34, 37, 40, 43, 47, 45, 54, 57, 60, 64, 67, 70, 73, 76,
                     78, 80, 82, 83, 84, 85, 85, 85, 84, 83, 82, 80, 78, 76, 73, 70, 67, 64, 60, 57, 54, 45, 47, 43, 40, 37, 34, 31, 28, 26, 23, 21, 19, 17, 16, 14, 13, 12, 11, 10,
                     9, 9, 8, 7, 7, 7, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5 };

  for (int i = gauss_size - 1; i >= 0; i--) {
    if (!shouldContinueMode("breathing")) return;
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.SetPixelColor(j, RgbColor((rgb[0] * gauss[i]) / 100, (rgb[1] * gauss[i]) / 100, (rgb[2] * gauss[i]) / 100));
    }
    strip.Show();
    delay(32);
  }
}

/* Brightness of the strip follows a single FFT band level. Degrades to dark strip with no audio source.
   TODO: replace hardcoded band index "2" with a configurable HA entity when audio source is added. */
void AV_mode(String fft_band) {
  int band_index = fft_band.toInt();
  if (band_index < 0 || band_index >= 10) return;

  float scaled_val     = constrain(bandLevels[band_index] / 100.0, 0.0, 3.0);
  float gamma_corrected = constrain(pow(scaled_val / 3.0, 0.4), 0.0, 1.0);
  if (gamma_corrected < 0.02) gamma_corrected = 0.0;

  RgbColor color((int)(rgb[0] * gamma_corrected), (int)(rgb[1] * gamma_corrected), (int)(rgb[2] * gamma_corrected));
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.SetPixelColor(i, color);
  }
  strip.Show();
}

/* Animates a rotating rainbow across the strip. Ignores rgb color. Checks HA for mode changes per frame. */
void spectrum_mode(int speed_delay) {
  static unsigned long last_update = 0;
  static uint16_t hue_offset = 0;

  if (millis() - last_update < speed_delay) return;
  last_update = millis();

  if (!shouldContinueMode("spectrum")) return;

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t hue = (i * 255 / NUM_LEDS) + (hue_offset >> 8);
    uint8_t r, g, b;

    if (hue < 85) {
      r = hue * 3;         g = 255 - hue * 3; b = 0;
    } else if (hue < 170) {
      hue -= 85;
      r = 255 - hue * 3;  g = 0;             b = hue * 3;
    } else {
      hue -= 170;
      r = 0;               g = hue * 3;       b = 255 - hue * 3;
    }

    strip.SetPixelColor(i, RgbColor(r, g, b));
  }

  strip.Show();
  hue_offset += 256;
}

#endif
