import asyncio

import aiohttp
import numpy as np
import sounddevice as sd

# Audio
SAMPLE_RATE = 44100
BLOCK_DURATION = 0.1
FFT_BANDS = 10

# Home Assistant
HA_BASE_URL = "http://192.168.50.68:8123"
HA_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJmZTcwM2VlZmMwMmU0YTAwYTk4NmE2NDNhODAxNDhiZCIsImlhdCI6MTc0Nzg3NjQyNiwiZXhwIjoyMDYzMjM2NDI2fQ.TtldgcSxnR3XE7Uzmky5Apj3MCtMrkLul4PsSokDFEQ"
ESP_IP_ENTITIES = [
    "sensor.desk_ip",
    "sensor.wall_ip",
]

# ESP
ESP_ENDPOINT = "http://{ip}/update_fft"


def compute_fft_bands(audio_data, sample_rate):
    """Computes 10-band FFT magnitudes from a mono audio block."""
    center_freqs = [31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]
    fft_result = np.fft.rfft(audio_data)
    fft_freqs = np.fft.rfftfreq(len(audio_data), 1 / sample_rate)
    magnitudes = np.abs(fft_result)

    band_edges = [0] + [
        (center_freqs[i] + center_freqs[i + 1]) / 2
        for i in range(len(center_freqs) - 1)
    ] + [sample_rate / 2]

    band_levels = []
    for i in range(len(center_freqs)):
        low, high = band_edges[i], band_edges[i + 1]
        indices = np.where((fft_freqs >= low) & (fft_freqs < high))[0]
        level = float(np.mean(magnitudes[indices])) if indices.size else 0.0
        band_levels.append(round(level, 4))

    return band_levels


async def fetch_esp_ips(session):
    """Fetches current ESP IP addresses from the LED ESP HA integration sensor entities."""
    headers = {"Authorization": f"Bearer {HA_TOKEN}", "Content-Type": "application/json"}
    ips = []
    for entity in ESP_IP_ENTITIES:
        async with session.get(f"{HA_BASE_URL}/api/states/{entity}", headers=headers) as resp:
            data = await resp.json()
            ips.append(data["state"].strip())
    return ips


async def send_to_esp(ip, band_levels, session):
    """Posts 10 FFT band levels to a single ESP32's /update_fft endpoint."""
    try:
        async with session.post(
            ESP_ENDPOINT.format(ip=ip),
            json={"band_levels": band_levels},
            timeout=1
        ) as resp:
            if resp.status != 200:
                print(f"Warning: {ip} responded with status {resp.status}")
    except Exception as e:
        print(f"Failed to send to {ip}: {e}")


async def main():
    async with aiohttp.ClientSession() as session:
        esp_ips = await fetch_esp_ips(session)
        print(f"Got IPs: {esp_ips}")

        with sd.InputStream(
            channels=1,
            samplerate=SAMPLE_RATE,
            blocksize=int(SAMPLE_RATE * BLOCK_DURATION)
        ) as stream:
            try:
                while True:
                    audio_block, _ = stream.read(int(SAMPLE_RATE * BLOCK_DURATION))
                    band_levels = compute_fft_bands(audio_block[:, 0], SAMPLE_RATE)
                    await asyncio.gather(*(send_to_esp(ip, band_levels, session) for ip in esp_ips))
            except KeyboardInterrupt:
                pass


if __name__ == "__main__":
    asyncio.run(main())
