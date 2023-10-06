from rpi_ws281x import Adafruit_NeoPixel, Color
import time
import socket
import select
from display_funcs import *

NUM_LEDS = 300
LED_PIN = 12 # GPIO pin 12 - PWM0 for RPi 4
LED_FREQ_HZ = 800000
LED_DMA = 10
LED_BRIGHTNESS = 255
LED_INVERT = False   # True to invert the signal (when using NPN transistor level shift).
LED_CHANNEL = 0

# variables for AV
# SAMPLES = 256
# SAMPLING_FREQUENCY = 10000
# AMPLITUDE = 200

# gauss brightness values
gauss = [5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 8, 9, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 26, 28, 31, 34, 37, 40, 43, 47, 45, 54, 57, 60, 64, 67, 70, 73, 76, 78, 80, 82, 83, 84, 85, 85, 85, 84, 83, 82, 80, 78, 76, 73, 70, 67, 64, 60, 57, 54, 45, 47, 43, 40, 37, 34, 31, 28, 26, 23, 21, 19, 17, 16, 14, 13, 12, 11, 10, 9, 9, 8, 7, 7, 7, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5]

# initialise strip object
strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
strip.begin()

current_state = {
    'color_hex': '#010101',
    'display_type': 'solid'
}

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('localhost', 5555))
server_socket.listen(5)

def handle_client(client_socket):
    try:
        data = client_socket.recv(1024).decode('utf-8')

        if data:
            current_state['color_hex'], current_state['display_type'] = data.split()

    except Exception as e:
        print('Exception detected, exiting...', flush=True)
        print(e, flush=True)
        client_socket.close()

def hex_to_tuple(color_hex):
    color_hex = color_hex.lstrip('#')
    color_tuple = tuple(int(color_hex[i:i+2], 16) for i in (0, 2, 4))
    return color_tuple

while True:

    readable, _, _ = select.select([server_socket], [], [], 1.0)  # timeout 1 second

    for sock in readable:
        if sock == server_socket:
            client_socket, _ = server_socket.accept()
            print(f"Connection from {client_socket.getpeername()}")
            handle_client(client_socket)        

    if current_state['display_type'] == 'solid':

        [strip.setPixelColor(led, Color(*hex_to_tuple(current_state['color_hex']))) for led in range(NUM_LEDS)]
        strip.show()
        time.sleep(1/1000)

    elif current_state['display_type'] == 'breathing':

        [strip.setPixelColor(led, Color(*hex_to_tuple(current_state['color_hex']))) for led in range(NUM_LEDS)]
        for val in gauss:
            strip.setBrightness(val)
            strip.show()
            time.sleep(50/1000)

    elif current_state['display_type'] == 'spectrum':

        spectrum_mode(50, NUM_LEDS, strip)