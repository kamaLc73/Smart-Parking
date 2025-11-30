# Smart Parking System (ESP32)

A simple microcontroller-based smart parking system built for the ESP32 using ultrasonic sensors and LEDs to display occupancy for multiple parking spots. It also publishes status via MQTT (HiveMQ Cloud in the example).

---

## Features

- 4 parking spots using ultrasonic distance sensors
- LED indicators for each spot (green = free, red = occupied)
- MQTT publishing of parking status (JSON payload)
- Built with PlatformIO (Espressif 32 / Arduino framework)
- Wokwi simulation supported (wokwi.toml included)

---

## Hardware

- ESP32 (board: `esp32dev`)
- 4x Ultrasonic sensors (e.g., HC-SR04)
- 8x LEDs (4 green, 4 red) + resistors
- Jumper wires and breadboard

Wiring is defined in `src/main.cpp` as TRIG/ECHO & LED pin definitions.

---

## Configuration

Edit `src/main.cpp` and update the following (for local testing or Wokwi simulation):

- `WIFI_SSID` and `WIFI_PASSWORD` — your WiFi network credentials
- `MQTT_BROKER`, `MQTT_PORT`, `MQTT_USERNAME`, `MQTT_PASSWORD` — MQTT broker credentials (the example uses HiveMQ Cloud)

> Tip: For production / safety, move private credentials to a configuration file or environment variables rather than embedding them directly in the source.

---

## Build & Upload (PlatformIO)

Install PlatformIO or use Visual Studio Code with the PlatformIO extension.

Open the project root in the terminal and run:

```powershell
# Build the project
pio run

# Upload to the ESP32 (flash)
pio run -e esp32dev -t upload

# Serial monitor
pio device monitor -e esp32dev -b 115200
```

---

## Simulation (Wokwi)

This repo includes `wokwi.toml` for Wokwi simulation, which can be launched on [Wokwi](https://wokwi.com) or by importing the project into the Wokwi interface.

---

## MQTT

Status is published to the topic `smartparking/status` as JSON (see `src/main.cpp` for payload format). The project also subscribes to `smartparking/control` for possible remote controls.

---

## Libraries & Dependencies

- PlatformIO environment: `esp32dev` (`platform = espressif32`) in `platformio.ini`
- PubSubClient (`knolleary/PubSubClient`)
