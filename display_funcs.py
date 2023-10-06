from rpi_ws281x import Color
import time

def wheel(WheelPos):
    if WheelPos < 85:
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0)
    elif WheelPos < 170:
        WheelPos -= 85
        return Color(255 - WheelPos * 3, 0, WheelPos * 3)
    else:
        WheelPos -= 170
        return Color(0, WheelPos * 3, 255 - WheelPos * 3)

def spectrum_mode(SpeedDelay, NUM_LEDS, strip):
    for j in range(256 * 5):  # 5 cycles of all colors on wheel
        for i in range(NUM_LEDS):
            color = wheel(((i * 256 // NUM_LEDS) + j) & 255)
            strip.setPixelColor(i, color)
        strip.show()
        time.sleep(SpeedDelay / 1000.0)