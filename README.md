# HydroFlow

ESP32 web panel for switching four water pumps on and off through a 4-channel relay board.

The ESP32 joins your Wi-Fi and serves a control page styled like an industrial pump schematic. Each pump has its own start/stop control, and there are START ALL / STOP ALL buttons. The page re-reads the real relay states from the board every 2.5 seconds, so several phones or browsers stay in sync, and it shows when the link to the board is lost.

All pumps are forced off at boot.

## Hardware

- ESP32 dev board
- 4-channel relay module (active-low by default)
- Up to four pumps on the relay contacts, with their own power supply

| Pump | Relay channel | ESP32 GPIO |
| --- | --- | --- |
| 1 | K1 | 14 |
| 2 | K2 | 27 |
| 3 | K3 | 26 |
| 4 | K4 | 25 |

If your relay board is active-high (pumps run when they should be off), swap `RELAY_ON` and `RELAY_OFF` near the top of the sketch.

## Stack

- Arduino C++ on the ESP32 core (`WiFi`, `WebServer`), no extra libraries
- Control page is HTML/CSS/JS embedded in the sketch

## Setup

1. Copy `wifi_secrets.example.h` to `wifi_secrets.h` in the same folder and fill in your Wi-Fi name and password (`HOME_WIFI_SSID`, `HOME_WIFI_PASS`). Keep `wifi_secrets.h` out of Git.
2. The sketch file is `sketch_sep29a.ino`, so the folder it sits in must also be named `sketch_sep29a` for Arduino to open it. If you cloned this repo under another name, rename the folder or clone with `git clone <repo-url> sketch_sep29a`.

## Flash

Arduino IDE: install the "esp32 by Espressif Systems" board package, open `sketch_sep29a.ino`, select your board and port, and upload.

arduino-cli:

```bash
arduino-cli core install esp32:esp32
arduino-cli compile --fqbn esp32:esp32:esp32 sketch_sep29a
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 sketch_sep29a
```

Open Serial Monitor at 115200 baud. Once it connects it prints its IP address; open that address in a browser on the same network.

## HTTP API

- `GET /` - control page
- `GET /pump?id=1&state=1` - set one pump (id 1-4, state 0 or 1); returns `1` or `0`
- `GET /all?state=0` - set all pumps
- `GET /status` - JSON array of the four pump states, e.g. `[true,false,false,false]`

There is no authentication. Anyone on the same network can switch the pumps.
