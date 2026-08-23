# Weather Jar

A small ESP32-S3 weather display that represents the current weather using light, atmospheric effects, and optional physical effects such as mist, rain, and lightning.

The first working version focuses on the WS2812 LED ring, with support for sunrise/sunset transitions and an HTTP API for status, configuration, and testing.

## Features

- ESP32-S3 based
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
- Optional dedicated lightning LED
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

the optional lightning LED can generate random flashes.

A lightning event can contain multiple flashes with random:

- intensity
- flash duration
- delay between flashes
- delay between lightning events

The feature can be enabled or disabled in `config.h`.

---

## Hardware

Current / planned components:

- ESP32-S3
- WS2812B LED ring
- optional dedicated lightning LED
- mini submersible water pump
- ultrasonic mist maker
- external power supply as required

The LED ring is currently the primary output device.

The pump and mist maker are represented in the configuration and hardware setup but their weather behaviour can be implemented independently.

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

ENABLE_WEATHER_LIGHT
ENABLE_SOLAR_EFFECTS
ENABLE_LIGHTNING_LED
ENABLE_PUMP
ENABLE_MISTER

WEATHER_UPDATE_INTERVAL
LED_RING_UPDATE_INTERVAL

SOLAR_EFFECT_HALF_WINDOW_SECONDS
```

It also contains weather brightness values and lightning timing configuration.

### `secrets.h`

Contains private configuration such as Wi-Fi credentials.

Example:

```cpp
#pragma once

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
```

Do not commit real credentials to a public repository.

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
    "brightness": 0
  }
}
```

`source` can be:

```text
real
mock
```

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
    "pin": 6
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
ESP32-S3
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
ESP32-S3
    ├── LED ring
    ├── lightning LED
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