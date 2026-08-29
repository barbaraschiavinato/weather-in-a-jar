# Weather Jar

A small ESP32 weather display that represents the current weather using light, atmospheric effects, and optional physical effects such as mist, rain, and lightning.

The first working version focuses on the WS2812 LED ring, with support for sunrise/sunset transitions and an HTTP API for status, configuration, and testing.

## Hardware Requirements

Core Components
* **1x ESP32 Quad MOS Board:** Development board based on the **ESP32-32E** microcontroller, featuring 4 integrated N-channel MOSFET channels. Ideal for controlling DC loads up to 60V silently, without the clicking noise of mechanical relays.
* **1x WS2812B RGB LED Ring (35 LEDs):** 5V addressable LED ring used to simulate weather colors, sunrises, and sunsets.
* **1x Mini Submersible Water Pump (5V DC):** Used to physically simulate rain inside the jar.
* **1x Mini Ultrasonic Mist Maker (5V DC):** Used to generate atmospheric fog/mist based on real-time weather conditions.
* **2x Standard 5V LEDs (White/Cool White):** Dedicated high-intensity light sources used to simulate quick lightning flashes during thunderstorms.

### 🔌 Power Supply & Wiring (Solderless Setup)
* **1x Raspberry Pi 5 USB-C Power Adapter (27W - 5.1V / 5A):** Required to deliver the high current (approx. 3.5A peak) needed when the LED ring (at full white brightness), the water pump, and the mist maker run simultaneously.
* **1x USB-C Female to 5-Pin Screw Terminal Adapter:** Essential to securely connect the Raspberry Pi 5 power supply without cutting its original cable. The 5-pin version includes built-in 5.1kΩ pull-down resistors required to trigger power delivery from smart USB-PD power bricks.
* **Jumper / Hookup Wires:** Standard electrical wire (AWG22 or AWG24 recommended for main power lines) to link the screw terminals together.
* **1x Glass Jar or Lantern:** The enclosure housing the electronics and containing the physical atmospheric effects.

Pin Mapping (Optimized for ESP32 Quad MOS)
When using the ESP32 Quad MOS board, modify the pin definitions in the `config.h` file as follows to match the onboard hardwired MOSFETs and prevent hardware conflicts:

| Component | ESP32 GPIO Pin | Physical Connection on the Board |
| :--- | :---: | :--- |
| **RGB LED Ring (Data In)** | `GPIO 13` | Free GPIO 13 Pin Header (Power bypassed directly to power source) |
| **Mini Water Pump** | `GPIO 16` | **OUT1** Screw Terminal (MOSFET 1) |
| **Mini Mist Maker** | `GPIO 17` | **OUT2** Screw Terminal (MOSFET 2) |
| **Lightning LEDs (x2)** | `GPIO 18` | **OUT3** Screw Terminal (MOSFET 3 - Wired in parallel) |


## Features

- ESP32 based
- Weather data from Open-Meteo
- Automatic Wi-Fi connection
- Automatic time synchronisation via NTP
- UK timezone with automatic GMT/BST handling
- WS2812B LED ring
- Weather-dependent LED colours
- Weather-dependent brightness
- Sunrise animation
- Sunset animation
- Different solar effects depending on cloud coverage
- Night mode
- Optional dedicated lightning LEDs (up to two)
- Mock API for testing weather conditions
- Accelerated sunrise/sunset simulation
- HTTP status endpoint
- HTTP configuration endpoint
- Future support for:
  - water pump
  - mist maker

---

## Weather Representation

The current weather is converted from the Open-Meteo weather code into one of the following internal states:

| Weather | Internal value |
|---|---|
| Clear | `clear` |
| Mainly clear | `mainly_clear` |
| Partly cloudy | `partly_cloudy` |
| Overcast | `overcast` |
| Fog | `fog` |
| Drizzle | `drizzle` |
| Rain | `rain` |
| Snow | `snow` |
| Thunderstorm | `thunderstorm` |

The weather controls the base colour and brightness of the LED ring.

Examples:

- Clear → warm white
- Mainly clear → warm cream
- Partly cloudy → neutral warm white
- Overcast → grey
- Fog → pale grey
- Drizzle → light blue
- Rain → blue
- Snow → cold white/blue
- Thunderstorm → dark blue

---

## Sunrise and Sunset

The Weather Jar also uses sunrise and sunset information returned by Open-Meteo.

The day is divided into four periods:

- `night`
- `sunrise`
- `day`
- `sunset`

The duration of the sunrise and sunset effect is controlled by:

```cpp
SOLAR_EFFECT_HALF_WINDOW_SECONDS
```

The complete effect duration is:

```text
SOLAR_EFFECT_HALF_WINDOW_SECONDS × 2
```

For example, with:

```cpp
SOLAR_EFFECT_HALF_WINDOW_SECONDS = 1800;
```

the transition lasts one hour:

```text
30 minutes before sunrise
        ↓
      sunrise
        ↓
30 minutes after sunrise
```

The same behaviour applies to sunset.

---

## Solar Visibility

Sunrise and sunset behaviour also depends on the current weather.

### Full visibility

Used for:

- clear
- mainly clear

The LED ring moves through a full sunrise/sunset colour sequence.

Example sunrise:

```text
OFF
↓
deep red
↓
red
↓
orange
↓
amber
↓
warm white
↓
current weather colour
```

Sunset performs the reverse transition.

### Partial visibility

Used for:

- partly cloudy

The sunrise/sunset colours are softer and more muted.

### No visibility

Used for:

- overcast
- fog
- drizzle
- rain
- snow
- thunderstorm

Instead of displaying strong sunrise colours, the normal weather colour simply fades in or fades out.

This means, for example, that a rainy sunrise does not suddenly turn the jar orange.

---

## Night Mode

Outside the sunrise/day/sunset periods, the LED ring is switched off.

```text
brightness = 0
RGB = 0, 0, 0
```

---

## Lightning

When the effective weather state is:

```text
thunderstorm
```

the optional lightning system can generate random flashes.

The first lightning LED is the primary flash source. A second optional LED can be enabled to reproduce the same flash with a configurable delay, creating a more natural sense of propagation through the jar.

A lightning event can contain multiple flashes with random:

- intensity
- flash duration
- delay between flashes
- delay between lightning events

The second LED:

- can be enabled or disabled independently
- uses a separate GPIO pin
- mirrors the brightness and duration of the first LED
- can be delayed relative to the first LED
- is controlled without blocking the main loop

Typical configuration:

```cpp
LIGHTNING_LED_PIN
LIGHTNING_LED_2_PIN

ENABLE_LIGHTNING_LED
ENABLE_LIGHTNING_LED_2

LIGHTNING_LED_2_DELAY_MS
```

For example:

```cpp
constexpr bool ENABLE_LIGHTNING_LED_2 = true;
constexpr unsigned long LIGHTNING_LED_2_DELAY_MS = 35;
```

With a delay of `0`, both LEDs flash together.

With a small delay such as `20-60 ms`, the second LED appears slightly after the first and can make the lightning effect feel more spatial.

The lightning mock uses the same lightning engine as real thunderstorm weather, so the second LED and its delay can be tested through the HTTP API.

---

## Hardware

Current / planned components:

- ESP32
- WS2812B LED ring
- up to two optional dedicated lightning LEDs
- mini submersible water pump
- ultrasonic mist maker
- external power supply as required

The LED ring is currently the primary output device.

The pump and mist maker are represented in the configuration and hardware setup but their weather behaviour can be implemented independently.

---

## Power and Wiring Architecture

The Weather Jar uses a single USB-C power source split into three branches.

```text
USB-C POWER SPLITTER
│
├── USB-C #1 → ESP32
│
├── USB-C #2 → 5V/GND adapter
│               └── WS2812 ring
│
└── USB-C #3 → 5V/GND adapter
                └── MOSFET BOARD
                    ├── CH1 → lightning LED 1
                    ├── CH2 → pump
                    ├── CH3 → mister
                    └── CH4 → lightning LED 2
```

### ESP32 GPIO connections

```text
ESP32
│
├── GPIO 18  LED_RING_PIN
│      └──────────────→ DIN of the WS2812 ring
│
├── GPIO 23  LIGHTNING_LED_PIN
│      └──────────────→ CH1 MOSFET
│
├── GPIO 19  PUMP_PIN
│      └──────────────→ CH2 MOSFET
│
├── GPIO 21  MISTER_PIN
│      └──────────────→ CH3 MOSFET
│
└── GPIO 22  LIGHTNING_LED_2_PIN
       └──────────────→ CH4 MOSFET
```

### MOSFET channels

| Channel | Device | ESP32 control |
|---|---|---|
| CH1 | Lightning LED 1 | GPIO 23 |
| CH2 | Water pump | GPIO 19 |
| CH3 | Mist maker | GPIO 21 |
| CH4 | Lightning LED 2 | GPIO 22 |

The second lightning LED is optional in software and is controlled independently through `LIGHTNING_LED_2_PIN`. Its flash can be delayed relative to the first LED using `LIGHTNING_LED_2_DELAY_MS`.

### WS2812 ring

The WS2812 ring is powered independently from USB-C #2:

```text
USB-C #2
    │
    └── 5V/GND adapter
            │
            ├── 5V  → WS2812 5V
            └── GND → WS2812 GND

ESP32 GPIO 18 ─────→ WS2812 DIN
```

The ring therefore does not draw its main power through the ESP32. The ESP32 only provides the data signal on `GPIO 18`.

### Lightning LEDs

Both lightning LEDs are powered through the MOSFET board:

```text
USB-C #3
    │
    └── 5V/GND adapter
            │
            └── MOSFET BOARD
                  │
                  ├── CH1 → lightning LED 1
                  │          ↑
                  │       GPIO 23
                  │
                  └── CH4 → lightning LED 2
                             ↑
                          GPIO 22
```

LED 1 is the primary lightning output. LED 2 is optional and mirrors each flash with the configured delay.

---

## Files

Typical project structure:

```text
mini_weather_central/
├── mini_weather_central.ino
├── config.h
├── secrets.h
└── types.h
```

### `mini_weather_central.ino`

Main application.

Contains:

- Wi-Fi setup
- time synchronisation
- Open-Meteo requests
- weather mapping
- LED calculations
- sunrise/sunset effects
- lightning logic
- HTTP API
- mock system
- main loop

### `config.h`

Contains hardware and behaviour configuration such as:

```cpp
LED_RING_PIN
LED_RING_COUNT

PUMP_PIN
MISTER_PIN
LIGHTNING_LED_PIN
LIGHTNING_LED_2_PIN

ENABLE_WEATHER_LIGHT
ENABLE_SOLAR_EFFECTS
ENABLE_LIGHTNING_LED
ENABLE_LIGHTNING_LED_2
ENABLE_PUMP
ENABLE_MISTER

LIGHTNING_LED_2_DELAY_MS

WEATHER_UPDATE_INTERVAL
LED_RING_UPDATE_INTERVAL

SOLAR_EFFECT_HALF_WINDOW_SECONDS
```

It also contains weather brightness values and lightning timing configuration.

### `secrets.h`

Contains private/local configuration such as Wi-Fi credentials and the location used for weather data.

Example:

```cpp
#pragma once

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

// Location
constexpr float LATITUDE = 51.51147;
constexpr float LONGITUDE = -0.13078308;
```

### Finding latitude and longitude

The coordinates should identify the location for which the Weather Jar retrieves weather information.

An easy way to find them is with Google Maps:

1. Open Google Maps.
2. Find the location you want the Weather Jar to represent.
3. Right-click the exact point on the map.
4. The latitude and longitude appear at the top of the context menu.
5. Click the coordinates to copy them.
6. Put the first value in `LATITUDE` and the second value in `LONGITUDE`.

For example, coordinates shown as:

```text
51.51147, -0.13078308
```

become:

```cpp
constexpr float LATITUDE = 51.51147;
constexpr float LONGITUDE = -0.13078308;
```

Latitude is always the first value and longitude is the second.

Negative values are significant and must be preserved. For example, locations west of the Greenwich meridian normally have a negative longitude.

The coordinates do not need to identify your exact home address. Coordinates for your town, neighbourhood, or another nearby representative point are normally sufficient for the Weather Jar.

Do not commit real Wi-Fi credentials to a public repository. If the repository is public and you consider your precise location private, use approximate coordinates or keep the location values out of the committed file as well.

### `types.h`

Contains the shared enums and state structures:

```text
WeatherType
DayPeriod
SunVisibility
RgbColor
WeatherState
MockState
LedState
LightningState
```

---

## Weather Source

Weather data is retrieved from Open-Meteo.

The application requests:

```text
temperature_2m
weather_code
precipitation
rain
sunrise
sunset
```

The request uses the latitude and longitude configured in `config.h`.

---

## HTTP API

Once the ESP32 connects to Wi-Fi, its IP address is printed to the Serial Monitor.

Example:

```text
IP address: 192.168.1.123
```

The API is then available at:

```text
http://192.168.1.123/
```

---

## Status

```http
GET /api/status
```

Returns the current effective state.

Example:

```json
{
  "wifi": "connected",
  "ip": "192.168.1.123",
  "source": "real",
  "weather": "partly_cloudy",
  "period": "day",
  "sunVisibility": "partial",
  "temperature": 23.7,
  "weatherCode": 2,
  "precipitation": 0,
  "rain": 0,
  "ring": {
    "enabled": true,
    "effect": "partly_cloudy",
    "progress": 0,
    "brightness": 180,
    "r": 220,
    "g": 220,
    "b": 205
  },
  "lightning": {
    "enabled": true,
    "active": false,
    "brightness": 0,
    "led2Enabled": true,
    "led2DelayMs": 35,
    "brightness2": 0
  }
}
```

`source` can be:

```text
real
mock
```

The `lightning` object also exposes the runtime state of the optional second lightning LED:

- `led2Enabled` — whether the second LED is enabled
- `led2DelayMs` — configured delay relative to the first LED
- `brightness2` — current output brightness of the second LED

---

## Configuration

```http
GET /api/config
```

Returns the hardware and runtime configuration currently used by the device.

Example:

```json
{
  "latitude": 51.51147,
  "longitude": -0.130783,
  "weatherUpdateSeconds": 900,
  "solarEffectWindowMinutes": 60,
  "ring": {
    "enabled": true,
    "pin": 5,
    "count": 32,
    "updateIntervalMs": 100
  },
  "lightning": {
    "enabled": true,
    "pin": 6,
    "led2Enabled": true,
    "led2Pin": 22,
    "led2DelayMs": 35
  },
  "pump": {
    "enabled": false,
    "pin": 7
  },
  "mister": {
    "enabled": false,
    "pin": 8
  }
}
```

Values depend on `config.h`.

---

## Mock API

The mock API allows the Weather Jar to be tested without waiting for real weather conditions.

### Mock weather

```http
GET /api/mock?weather=clear
GET /api/mock?weather=mainly_clear
GET /api/mock?weather=partly_cloudy
GET /api/mock?weather=overcast
GET /api/mock?weather=fog
GET /api/mock?weather=drizzle
GET /api/mock?weather=rain
GET /api/mock?weather=snow
GET /api/mock?weather=thunderstorm
```

When thunderstorm mode is enabled, lightning starts using the normal lightning engine. If the second lightning LED is enabled, it follows the first LED using the configured delay.

Example:

```http
GET /api/mock?weather=rain
```

The device continues using the real time period but displays rain.

---

## Mock Day Period

A time period can also be forced:

```http
GET /api/mock?period=night
GET /api/mock?period=sunrise
GET /api/mock?period=day
GET /api/mock?period=sunset
```

Example:

```http
GET /api/mock?period=sunset
```

---

## Combined Mocks

Weather and day period can be combined.

Examples:

```http
GET /api/mock?weather=clear&period=sunrise
```

```http
GET /api/mock?weather=partly_cloudy&period=sunset
```

```http
GET /api/mock?weather=rain&period=sunrise
```

This is particularly useful for checking how cloud coverage changes the solar effects.

---

## Accelerated Sunrise / Sunset

Normally the solar animation follows real time.

For testing, the animation can be accelerated using:

```text
speed
```

Example:

```http
GET /api/mock?weather=clear&period=sunrise&speed=60
```

A normal one-hour animation will therefore complete approximately 60× faster.

Other examples:

```http
GET /api/mock?weather=clear&period=sunset&speed=60
```

```http
GET /api/mock?weather=partly_cloudy&period=sunrise&speed=120
```

---

## Temporary Mocks

A mock can automatically expire using:

```text
duration
```

The value is expressed in seconds.

Example:

```http
GET /api/mock?weather=thunderstorm&duration=60
```

The Weather Jar will simulate a thunderstorm for 60 seconds and then automatically return to real weather.

---

## Disable Mock Mode

```http
GET /api/mock/off
```

This immediately restores:

- real weather
- real day period
- normal sunrise/sunset timing

---

## Useful Test URLs

Assuming the ESP32 address is:

```text
192.168.1.123
```

### Current status

```text
http://192.168.1.123/api/status
```

### Configuration

```text
http://192.168.1.123/api/config
```

### Clear sky

```text
http://192.168.1.123/api/mock?weather=clear
```

### Rain

```text
http://192.168.1.123/api/mock?weather=rain
```

### Thunderstorm

```text
http://192.168.1.123/api/mock?weather=thunderstorm
```

### Fast clear sunrise

```text
http://192.168.1.123/api/mock?weather=clear&period=sunrise&speed=60
```

### Fast clear sunset

```text
http://192.168.1.123/api/mock?weather=clear&period=sunset&speed=60
```

### Fast partly cloudy sunrise

```text
http://192.168.1.123/api/mock?weather=partly_cloudy&period=sunrise&speed=60
```

### Rainy sunrise

```text
http://192.168.1.123/api/mock?weather=rain&period=sunrise&speed=60
```

### Restore real mode

```text
http://192.168.1.123/api/mock/off
```

---

## Serial Monitor

Recommended baud rate:

```text
115200
```

Typical startup output:

```text
==========================
WEATHER JAR BOOT
==========================

Initialising FastLED...
FastLED ready.

Connecting to Wi-Fi...
Wi-Fi connected.
IP address: 192.168.1.123

Synchronising time...
Time synchronised.

Fetching weather...

==========================
Temperature: 23.70 C
Weather code: 2
Precipitation: 0.00 mm
Rain: 0.00 mm

WEATHER: partly_cloudy
==========================

HTTP server started.
Applying initial weather light...
Initial weather light applied.

==========================
SETUP COMPLETE
==========================
```

---

## Libraries

The project currently uses:

```text
WiFi
WebServer
HTTPClient
ArduinoJson
FastLED
time
```

External libraries that need to be installed through Arduino Library Manager:

- ArduinoJson
- FastLED

The ESP32 board package provides the remaining ESP32-specific libraries.

---

## Development Roadmap

Current:

```text
Weather API
    ↓
ESP32
    ↓
Weather interpretation
    ↓
Sunrise / sunset interpretation
    ↓
WS2812 LED ring
```

Planned:

```text
Weather API
    ↓
ESP32
    ├── LED ring
    ├── lightning LED 1
    ├── lightning LED 2 (optional, delayed)
    ├── mist maker
    └── water pump
```

Possible future improvements include:

- intermittent mist for clouds
- rain simulation using the pump
- precipitation intensity mapping
- lightning frequency based on storm severity
- smoother weather transitions
- local web interface
- persistent configuration
- OTA firmware updates
- Home Assistant integration
