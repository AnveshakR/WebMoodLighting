void solid_mode()
{
  for (int i = 0; i < NUM_LEDS; i++) 
    {
      if(val_change == true) {val_change = false;break;}

      strip.SetPixelColor(i, RgbColor(r_val*0.85, g_val*0.85, b_val*0.85));
    }
    strip.Show();
    delay(1);
}

void breathing_mode()
{
  for (int i = gauss_size - 1; i >= 0; i--) {

    if(val_change == true) {val_change = false;break;}

    for (int j = 0; j < NUM_LEDS; j++) 
      {
        strip.SetPixelColor(j, RgbColor((r_val * gauss[i]) / 100, (g_val * gauss[i]) / 100, (b_val * gauss[i]) / 100));
      }
      strip.Show();
      delay(32);
    }
}

void basicAV_mode()
{
  for (int i = 0; i < SAMPLES; i++) 
  {

    if(val_change == true) {val_change = false;break;}

    newTime = micros()-oldTime;
    oldTime = newTime;
    vReal[i] = analogRead(A0); // A conversion takes about 1mS on an ESP8266
    vImag[i] = 0;
    while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  for (int i = 2; i < (SAMPLES/2); i++)
  {
    if (vReal[i] > amplitude)
      {
        scale = (vReal[i]/amplitude)/100.0;
        if (scale>1.0)
        {
          scale = 1.0;
        }
        if (scale<0.05)
        {
          scale = 0.05;
        }
        // Serial.println(0);
        // Serial.println(1);
        // Serial.println(scale);
        for (int j=0; j< NUM_LEDS ; j++)
        {
          strip.SetPixelColor(j, RgbColor(r_val*scale, g_val*scale, b_val*scale));
        }
        strip.Show();
        delay(1);
      }
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];
 
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}

void spectrum_mode(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< NUM_LEDS; i++) 
    {

      if(val_change == true) {val_change = false;break;}

      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      strip.SetPixelColor(i, RgbColor(int(*c), int(*(c+1)), int(*(c+2))));
    }
    strip.Show();
    delay(SpeedDelay);
  }
}