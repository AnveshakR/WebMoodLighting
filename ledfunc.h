void solid_mode()
{
  for (int i = 0; i < NUM_LEDS; i++) 
    {
      strip.SetPixelColor(i, RgbColor(r_val*0.85, g_val*0.85, b_val*0.85));
    }
    strip.Show();
    delay(1);
}

void breathing_mode()
{
  for (int i = gauss_size - 1; i >= 0; i--) {

    if(val_change == true) {break;}

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