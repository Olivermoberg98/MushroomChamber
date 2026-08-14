# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A mushroom growing chamber: an ESP32 firmware node that reads a BME280 and drives fans/humidifier/LEDs, plus a Node + React server (intended to run on a Raspberry Pi) that collects readings and serves a dashboard. The two halves talk over plain HTTP on the LAN.

Three top-level pieces:

- `MushroomChamberController/` — PlatformIO/Arduino firmware for an ESP32 (`env:esp32dev`).
- `backend/` — Express server (`server.js`, port 3001) that receives sensor POSTs and serves the built frontend.
- `backend/vite-project/` — React 19 + Vite + Tailwind dashboard; a single component, [Dashboard.jsx](backend/vite-project/src/components/Dashboard.jsx).

## Commands

Firmware (run from `MushroomChamberController/`):

```bash
pio run                      # build
pio run -t upload            # build + flash
pio device monitor           # serial @ 115200
```

Backend (run from `backend/`):

```bash
npm run dev                  # server only, no frontend rebuild
npm run build                # build frontend into vite-project/dist
npm start                    # build frontend + start server
```

Frontend dev (run from `backend/vite-project/`):

```bash
npm run dev                  # Vite dev server
npm run lint                 # eslint
```

### Running firmware tests

Tests live in `MushroomChamberController/test/test_*/` and use Unity, but **every test environment in `platformio.ini` is commented out**. To run one, uncomment its `[env:esp32_*_test]` block, then:

```bash
pio test -e esp32_wifi_comm_test
```

Each block already carries the right `test_filter` and `lib_deps`. These are on-hardware tests — they need a connected ESP32; several (`test_wifi_*`) also need the backend reachable on the LAN. There is no host-side/native test environment.

## Architecture

### Control flow on the ESP32

[main.cpp](MushroomChamberController/src/main.cpp) is a plain `setup()`/`loop()` with a 2-second delay. Each iteration: read BME280 → `wifiRetryLoop()` → if connected, POST readings and GET the current phase → `updateActuators()` → `controlLighting()`.

State is held in **globals defined in `main.cpp`** and pulled in via `extern` elsewhere: `currentConfig` (the mushroom species), `currentPhase`, and `activePhaseConfig`. `config.cpp` and `actuators.cpp` both reach for these rather than taking parameters, so changing ownership of any of them touches multiple translation units.

### Configuration model

[mushroom_types.h](MushroomChamberController/src/mushroom_types.h) defines the shape: a `MushroomConfig` is a species name plus three `PhaseConfig` structs (incubation, primordia, fruiting). A `PhaseConfig` carries temp/humidity/pressure targets with tolerances, a light on/off hour pair, and a `CRGB` color.

All species tables are hardcoded as struct literals in `getMushroomConfig()` in [config.cpp](MushroomChamberController/src/config.cpp). **The literals are positional** — field order must match `PhaseConfig` exactly, and adding a field means editing all seven species blocks. The species is selected once in `setup()` (`getMushroomConfig(SHIITAKE)`); there is no runtime way to change it.

Growth phase, by contrast, *is* runtime-controlled: the server owns it, and the ESP32 polls `/api/phase` every loop.

### Phase strings must stay in sync across three places

The phase names `"Incubation"`, `"Primordia"`, `"Fruiting"` are duplicated in:

1. `phaseConfigs` in [server.js](backend/server.js) (validates POST `/api/phase`)
2. `stringToGrowthPhase()` / `growthPhaseToString()` in [wifi_comm.cpp](MushroomChamberController/src/wifi_comm.cpp)
3. the enum `GrowthPhase` in `mushroom_types.h` (`PRIMORDIA_FORMATION`, note the name mismatch with the wire string)

An unrecognized string silently falls back to `INCUBATION`.

### The adaptive actuator controller

[actuators.cpp](MushroomChamberController/src/actuators.cpp) is the most involved part of the firmware. It's a four-state machine — `HUMIDIFYING → STABILIZING → VENTILATING → RECOVERING` — layered under two emergency overrides (critical low humidity, critical high temp) that `return` early and bypass the state machine entirely.

It self-tunes: `humidifyDuration` is learned from how long humidification actually took, and `ventilationDuration` is nudged up or down based on the observed humidity drop per ventilation cycle. Humidity is exponentially filtered (`alpha = 0.2`) before any decision. `updateActuators()` rate-limits itself to once per second internally, so calling it more often is harmless.

Note it only reads `targetHumidity` from the active phase config — temperature and pressure targets/tolerances are currently unused by the controller; the thresholds it acts on (`criticalLowHumidity`, `criticalHighTemp`) are controller fields, not phase config.

### Server

`server.js` keeps everything **in memory** — latest reading, plus a rolling 100-entry `sensorHistory`. Restart loses all data; there is no database.

API surface: `POST /api/sensor-data` (from ESP32), `GET /api/data`, `GET /api/history`, `GET /api/status`, `GET|POST /api/phase`, `GET /api/phases`, `GET /api/uptime`, `POST /api/update-system`. Anything not matching `/api` falls through to `vite-project/dist/index.html`.

`POST /api/update-system` is a self-updater: it runs `git fetch`/`git pull origin main`, `npm install`, rebuilds the frontend, then calls `process.exit(0)` expecting **PM2** to restart it. It's triggered from a button in the dashboard.

`latestSensorData.timestamp` is always an ISO string or `null`. Both `/api/data` and `/api/status` report `esp32_connected` through the shared `isDataFresh()` helper — don't re-derive freshness inline, since comparing the raw ISO string against `Date.now()` yields `NaN` and silently reports "disconnected" forever.

## Things to know before editing

- **Network config is hardcoded in source.** SSID, password, and the server URL are literals in `main.cpp:31` and repeated in the `test_wifi_*` test files. Changing networks means editing all of them. There's no secrets file or build flag for this.
- **Pin assignments are `#define`s** at the top of the module that uses them: fans/humidifier in `actuators.cpp` (13, 12, 14, 15), LED strip in [led.h](MushroomChamberController/src/led.h) (pin 27, 60 LEDs), BME280 I2C address in `sensors.cpp` (0x76).
- **`backend/node_modules/` was committed for most of this repo's history** and was untracked in the working tree (see `backend/.gitignore`). Commits before that point contain ~5200 dependency files, so `git log`/`git blame` over old revisions is noisy, and the repo history is still large — filter with pathspecs like `git log -- backend/server.js` rather than scanning broadly.
- `test/test_wifi_to_server/` contains its own copies of `wifi_comm.cpp/.h` and `mushroom_types.h`, forked from `src/`. Edits to `src/` do not propagate there.
- The Vite dev server is configured for port 3001 — the same port the Express server binds. Running both means changing one. The server's CORS origin is set to `http://localhost:5173` (Vite's default), not 3001.
- Lighting depends on NTP: `setupTime()` runs after WiFi connects, hardcoded to GMT+1. If time sync fails, `controlLighting()` reads a garbage hour and the schedule is meaningless.
