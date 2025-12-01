# Smart Parking Dashboard — Docker

This guide explains how to build and run the React/Vite dashboard in a Docker container served by Nginx.

## Prerequisites
- Docker Desktop installed and running
- Optional: Docker Compose v2 (bundled with recent Docker Desktop)

## Quick Start

Option A — Compose (recommended):

```powershell
# From the repository root
docker compose up --build -d dashboard
docker compose logs -f dashboard
# Open http://localhost:8080

# Stop
docker compose down
```

Option B — Plain Docker:

```powershell
cd dashboard
# Build image
docker build -t smart-parking-dashboard .
# Run container mapped to host port 8080
docker run --rm -p 8080:80 smart-parking-dashboard
# Open http://localhost:8080
```

## Build-time Configuration (Vite envs)
The dashboard is a static SPA; environment variables are baked in at build time (variables prefixed with `VITE_`). You can provide them via Compose build args or by adding a `dashboard/.env.production` file before building.

Supported variables:
- `VITE_MQTT_BROKER`
- `VITE_MQTT_USERNAME`
- `VITE_MQTT_PASSWORD`
- `VITE_MQTT_TOPIC` (defaults to `smartparking/status`)

Using Compose with a project-level `.env` at the repository root (this file is used only for variable substitution and is not copied into the image):

```powershell
# Create .env at repo root
"VITE_MQTT_BROKER=wss://your-broker:8884/mqtt" | Out-File -Encoding ascii .env
"VITE_MQTT_USERNAME=your_user" | Add-Content .env
"VITE_MQTT_PASSWORD=your_password" | Add-Content .env
"VITE_MQTT_TOPIC=smartparking/status" | Add-Content .env

# Build and run
docker compose up --build -d dashboard
```

Using plain Docker build args:

```powershell
cd dashboard

docker build `
  --build-arg VITE_MQTT_BROKER=wss://your-broker:8884/mqtt `
  --build-arg VITE_MQTT_USERNAME=your_user `
  --build-arg VITE_MQTT_PASSWORD=your_password `
  --build-arg VITE_MQTT_TOPIC=smartparking/status `
  -t smart-parking-dashboard .

docker run --rm -p 8080:80 smart-parking-dashboard
```

## Notes
- For local dev, prefer `npm run dev` in `dashboard/`.
- Secrets in a static SPA are client-visible. Use public endpoints/credentials intended for browser clients.