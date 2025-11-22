# Terrarium Lighting Controller

ESP8266-based firmware that drives a terrarium light using PWM output with scheduling, timezone-aware automation, and a modern web dashboard. The device stores its configuration on LittleFS flash, exposes a captive portal for first-time Wi-Fi setup, and advertises itself on the LAN as `terrarium.local` via mDNS.

## Features
- Adjustable brightness with gamma-corrected PWM output for consistent low-level lighting
- Schedule window with on/off times, optional timezone auto-detection, and DST rules
- Responsive web UI served directly from the MCU (`/assets` bundle is baked into flash)
- Captive setup portal (`Terrarium-Setup/terra1234`) for provisioning Wi-Fi when credentials are missing
- Persistent storage in `/light_config.csv` on LittleFS
- Breathing status animation whenever the captive portal is active

## Building & Flashing
1. Install the **ESP8266 board package** inside the Arduino IDE.
2. Clone this repository and open `terrarium_fw.ino`.
3. Select your board and the correct serial port.
4. Ensure `Tools → Flash Size` leaves adequate LittleFS space (at least 1MB works well).
5. Compile and upload the sketch.
6. On first boot the firmware will format LittleFS if needed and create `/light_config.csv` with defaults.

Assets such as `main.css`, `app.js`, and HTML templates are included at compile time via `web_assets.cpp`, so no extra file upload step is required.

## Wi-Fi Provisioning & Access
- If no credentials are stored, the controller launches a captive portal. Join the `Terrarium-Setup` network (password `terra1234`) and browse to `http://192.168.4.1/` to enter your home SSID/password.
- Once connected to your LAN the device:
  - Logs the assigned IP on the serial console.
  - Starts an mDNS responder so you can browse to `http://terrarium.local/`.
- You can relaunch the portal from the web UI if you need to change networks.

## Web Interface
The dashboard exposes:
- Manual brightness slider with numeric entry
- Schedule configuration (on/off times, enable/disable checkbox)
- Timezone display plus automatic offset syncing from the browser
- Device stats (Wi-Fi status, clock sync, local time, portal state)

## Asset Embedding
Because `incbin` requires absolute paths, `web_assets.cpp` defines `ASSET_PATH_PREFIX` (defaulting to `/home/mike/source/terrarium_fw/assets/`). Update that constant to match your local repo path **before** compiling.
