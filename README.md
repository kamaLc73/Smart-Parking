# Smart Parking System (ESP32)

A simple microcontroller-based smart parking system built for the ESP32 Wokwi using ultrasonic sensors and LEDs to display occupancy for multiple parking spots. It also publishes status via MQTT (HiveMQ Cloud in the example).

---

## Table of Contents

- [Features](#features)
- [Hardware](#hardware)
- [Wiring (pinout)](#wiring-pinout)
- [Configuration](#configuration)
- [Build &amp; Upload (PlatformIO)](#build--upload-platformio)
- [Web Dashboard (React)](#web-dashboard-react)
- [Simulation (Wokwi)](#simulation-wokwi)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

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

## Wiring (pinout)

The pin mappings used in the firmware are listed below (defined in `src/main.cpp`):

| Place | TRIG | ECHO | LED GREEN | LED RED |
| ----- | ---- | ---- | --------- | ------- |
| 1     | 5    | 18   | 19        | 4       |
| 2     | 21   | 22   | 23        | 2       |
| 3     | 32   | 33   | 25        | 15      |
| 4     | 26   | 27   | 14        | 12      |

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

---

## Web Dashboard (React)

There is a sample React dashboard in the `dashboard/` folder that subscribes to the `smartparking/status` MQTT topic and displays parking places, statistics, and recent activity in real-time.

Quick start (from root):

```powershell
cd dashboard
npm install
npm run dev
```

Open the URL printed by Vite (default: `http://localhost:5173/`).

Configuration & Environment Variables:

- Node.js version: For best compatibility with dev dependencies (e.g., `@vitejs/plugin-react`), use Node.js >= 20.19.0 (or >= 22.12.0). Check your Node version with `node -v`.
- Instead of committing credentials directly into `src/ParkingDashboard.jsx`, use Vite environment variables. Create a `.env` file in `dashboard/` and add the public environment variables that start with `VITE_`, e.g.:

```powershell
cd dashboard
copy NUL .env
```

Then add to `.env`:

```env
VITE_MQTT_BROKER=wss://your-broker:8884/mqtt
VITE_MQTT_USERNAME=your_mqtt_user
VITE_MQTT_PASSWORD=your_mqtt_password
VITE_MQTT_TOPIC=smartparking/status
```

In `dashboard/src/ParkingDashboard.jsx`, you can read them like this:

```js
const MQTT_CONFIG = {
  broker: import.meta.env.VITE_MQTT_BROKER,
  username: import.meta.env.VITE_MQTT_USERNAME,
  password: import.meta.env.VITE_MQTT_PASSWORD,
  topic: import.meta.env.VITE_MQTT_TOPIC || 'smartparking/status',
};
```

Build & Production:

- Start dev server:

```powershell
cd dashboard
npm run dev
```

- Build (production):

```powershell
cd dashboard
npm run build
```

This generates a `dist/` folder which you can serve with any static server or deploy to a hosting provider.

Notes:

- The example uses the Paho MQTT client library (via CDN in `index.html`). To avoid relying on CDN in production, install an MQTT npm package (for example, `mqtt`) and import/initialize it from your React code.
- Use environment variables and secure WebSocket endpoints, and keep credentials out of version control (add `.env` to `.gitignore`).

---

## Troubleshooting

- npm install fails: try upgrading Node.js to the recommended version (≥ 20.19.0). Alternatively, adjust dependencies in `dashboard/package.json` to compatible versions.
- Vite ESM/CJS errors: ensure `dashboard/package.json` has `"type": "module"` and `vite.config.js` uses ESM import/export.
- No MQTT messages: verify the ESP32 publishes to `smartparking/status` and the dashboard is subscribed to the same broker and topic.

---

## Contributing

- Bug reports and pull requests are welcome. Please include reproduction steps and any wiring or hardware changes.
