# Weather Jar

A small ESP32-based ambient weather display that represents the current
weather with a WS2812B LED ring.

The current **working version is intentionally dry**: it uses only the
addressable LED ring as the physical weather output. The firmware
already provides weather interpretation, sunrise/sunset effects,
forecasts, HTTP control, MQTT/Home Assistant integration, and support
for the separate ESPHome touchscreen control panel.

> **Future improvement:** a second "wet" version may add a water pump,
> ultrasonic mist maker, and dedicated lightning LEDs. Those devices are
> not part of the hardware described in this README.

------------------------------------------------------------------------

## Current Architecture

``` text
                    Open-Meteo
                        │
                        ▼
                 ┌─────────────┐
                 │ Weather Jar │
                 │    ESP32    │
                 └──────┬──────┘
                        │
              GPIO 18 ──┴──► WS2812B ring
                        │
             HTTP API + MQTT
                  │           │
                  ▼           ▼
        ESPHome touch     Home Assistant
            panel             / MQTT
```

The Weather Jar is the source of truth for current weather and the
five-day forecast. The touchscreen panel does **not** query Open-Meteo
directly.

------------------------------------------------------------------------

# Hardware

## Weather Jar --- current dry version

### Required components

-   **1× ESP32 development board**
-   **1× WS2812B RGB LED ring --- 35 LEDs**
-   **1× USB-C power source**
-   **1× USB-C power splitter**
-   **5 V / GND breakout or screw-terminal adapter** for powering the
    LED ring independently
-   Jumper/hookup wires
-   Glass jar / lantern or other enclosure

### Power architecture

The ESP32 and LED ring are powered from separate branches of the same
USB supply:

``` text
USB-C POWER
    │
    ▼
USB-C SPLITTER
    │
    ├──► ESP32
    │
    └──► 5 V / GND breakout
              │
              └──► WS2812B ring
```

The ring does not draw its main current through the ESP32.

### WS2812B wiring

``` text
ESP32 GPIO 18 ─────────────► WS2812 DIN

USB 5 V ───────────────────► WS2812 5 V
USB GND ───────────────────► WS2812 GND
ESP32 GND ─────────────────► common GND
```

  Component     Connection
  ------------- ---------------------
  WS2812B DIN   GPIO 18
  WS2812B 5 V   External 5 V supply
  WS2812B GND   Common ground
  LED count     35

A common ground between the ESP32 and LED-ring supply is required for a
reliable data signal.

------------------------------------------------------------------------

# Optional Touchscreen Control Panel

The project also supports a separate ESPHome touchscreen controller.

Current hardware:

-   **Guition / Sunton JC8048W550 / JC8048W550C_I**
-   ESP32-S3
-   5-inch 800×480 RGB display
-   GT911 capacitive touchscreen
-   Wi-Fi connection to the same local network as the Weather Jar

The panel communicates directly with the Jar over its HTTP API.

It currently provides:

-   live weather view
-   five-day forecast
-   selectable forecast days
-   weather mock controls
-   sunrise/sunset test controls
-   brightness controls
-   location display
-   loading/activity indicator

The panel should use a **2.4 GHz Wi-Fi network**.

## Panel data behaviour

### LIVE

The large weather card displays current observations from:

``` http
GET /api/status
```

including:

-   current weather
-   current temperature
-   current period
-   precipitation
-   rain
-   location
-   user brightness

### TODAY

`TODAY` is deliberately different from `LIVE`.

It represents today's **daily forecast**, not the current observation,
and therefore displays:

-   today's forecast weather
-   today's maximum temperature
-   today's minimum temperature
-   `TODAY • FORECAST`

### Future days

The remaining forecast cards use the daily data returned by:

``` http
GET /api/forecast
```

Selecting one temporarily applies that forecast weather to the Jar for
preview/testing and displays the selected day's min/max values in the
large card.

------------------------------------------------------------------------

# Weather Source

Weather data is retrieved directly by the Weather Jar from Open-Meteo.

Current observations include:

``` text
temperature_2m
weather_code
precipitation
rain
```

Daily data includes:

``` text
weather_code
temperature_2m_max
temperature_2m_min
sunrise
sunset
```

The firmware requests five forecast days.

The weather request uses the latitude and longitude configured for the
Jar.

------------------------------------------------------------------------

# Weather States

Open-Meteo weather codes are mapped to these internal states:

  Display         Internal value
  --------------- -----------------
  Clear           `clear`
  Mainly clear    `mainly_clear`
  Partly cloudy   `partly_cloudy`
  Overcast        `overcast`
  Fog             `fog`
  Drizzle         `drizzle`
  Rain            `rain`
  Snow            `snow`
  Thunderstorm    `thunderstorm`

Each state controls the base colour and brightness of the LED ring.

Typical representation:

-   Clear → warm white
-   Mainly clear → warm cream
-   Partly cloudy → neutral warm white
-   Overcast → grey
-   Fog → pale grey
-   Drizzle → light blue
-   Rain → blue
-   Snow → cold white/blue
-   Thunderstorm → dark blue

------------------------------------------------------------------------

# Sunrise and Sunset

The Jar also uses sunrise and sunset information returned by Open-Meteo.

The day is divided into:

``` text
night
sunrise
day
sunset
```

The solar transition duration is controlled by:

``` cpp
SOLAR_EFFECT_HALF_WINDOW_SECONDS
```

The full transition lasts:

``` text
SOLAR_EFFECT_HALF_WINDOW_SECONDS × 2
```

For example, a value of 1800 seconds creates a one-hour transition
centred on sunrise or sunset.

## Solar visibility

The effect depends on cloud coverage.

### Full

Used for:

-   clear
-   mainly clear

The ring moves through the full sunrise/sunset colour sequence.

### Partial

Used for:

-   partly cloudy

Colours are softer and more muted.

### None

Used for:

-   overcast
-   fog
-   drizzle
-   rain
-   snow
-   thunderstorm

The normal weather colour fades in/out rather than showing strong solar
colours.

## Night mode

Outside the active sunrise/day/sunset periods, the ring is switched off.

------------------------------------------------------------------------

# User Brightness

The project separates **user brightness** from the instantaneous
brightness calculated by a weather effect.

User brightness is expressed as:

``` text
0–100 %
```

For example:

``` json
"brightnessPercent": 60
```

The effect engine can still report a different internal ring brightness.
This is expected: `brightnessPercent` is the user-selected multiplier,
while `ring.brightness` is the brightness currently calculated by the
active effect.

The touchscreen panel reads the user value directly from `/api/status`.

Brightness can also be changed directly over HTTP:

``` http
GET /api/brightness?action=up
GET /api/brightness?action=down
GET /api/brightness?value=60
```

The default increment/decrement step is 10%.

------------------------------------------------------------------------

# HTTP API

Once connected to Wi-Fi, the Jar exposes a local HTTP API.

Example base address:

``` text
http://192.168.1.212
```

Use the actual IP assigned to your Jar.

## Current status

``` http
GET /api/status
```

Example:

``` json
{
  "wifi": "connected",
  "ip": "192.168.1.212",
  "location": "London",
  "brightnessPercent": 60,
  "source": "real",
  "weather": "overcast",
  "period": "day",
  "sunVisibility": "none",
  "temperature": 21.6,
  "weatherCode": 3,
  "precipitation": 0,
  "rain": 0,
  "ring": {
    "enabled": true,
    "effect": "overcast",
    "progress": 0,
    "brightness": 110,
    "userBrightnessPercent": 60,
    "r": 95,
    "g": 110,
    "b": 125
  }
}
```

`source` is normally:

``` text
real
```

and becomes `mock` while a mock weather state is active.

## Five-day forecast

``` http
GET /api/forecast
```

Example structure:

``` json
{
  "valid": true,
  "count": 5,
  "days": [
    {
      "offset": 0,
      "date": "2026-08-31",
      "valid": true,
      "weather": "drizzle",
      "weatherCode": 53,
      "temperatureMax": 21.6,
      "temperatureMin": 14.9
    }
  ]
}
```

`offset: 0` is TODAY.

## Configuration

``` http
GET /api/config
```

The configuration response includes the configured location and
runtime/hardware settings.

## Brightness

``` http
GET /api/brightness
GET /api/brightness?action=up
GET /api/brightness?action=down
GET /api/brightness?value=100
```

------------------------------------------------------------------------

# Mock API

Mocks allow every visual state to be tested without waiting for matching
real weather.

## Weather

``` http
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

## Day period

``` http
GET /api/mock?period=night
GET /api/mock?period=sunrise
GET /api/mock?period=day
GET /api/mock?period=sunset
```

Weather and period can be combined:

``` http
GET /api/mock?weather=clear&period=sunrise
```

## Accelerated solar effects

``` http
GET /api/mock?weather=clear&period=sunrise&speed=60
GET /api/mock?weather=clear&period=sunset&speed=60
```

## Temporary mocks

``` http
GET /api/mock?weather=rain&duration=60
```

## Return to real weather

``` http
GET /api/mock/off
```

------------------------------------------------------------------------

# MQTT and Home Assistant

The Weather Jar connects to an MQTT broker and publishes Home Assistant
MQTT Discovery configuration automatically.

This allows Home Assistant to discover the Jar as a device rather than
requiring every entity to be created manually.

## MQTT topics

### Mode

``` text
Command: weatherjar/set/mode
State:   weatherjar/state/mode
```

### Brightness

``` text
Command: weatherjar/set/brightness
State:   weatherjar/state/brightness
```

### Forecast override

``` text
Command: weatherjar/set/forecast
State:   weatherjar/state/forecast
```

### Availability

``` text
weatherjar/status/availability
```

## Home Assistant MQTT Discovery

The firmware publishes discovery configuration under:

``` text
homeassistant/select/weatherjar_mode/config
homeassistant/number/weatherjar_brightness/config
homeassistant/binary_sensor/weatherjar_online/config
```

Home Assistant can therefore expose:

-   **Mode** --- select entity
-   **Brightness** --- number/slider entity
-   **Online** --- connectivity binary sensor

The discovered device is identified as:

``` text
Weather Jar
```

with model:

``` text
ESP32 Weather Jar
```

## Mode entity

The MQTT mode selector supports:

``` text
auto
clear
mainly_clear
partly_cloudy
overcast
fog
drizzle
rain
snow
storm
sunrise
sunset
loop
off
```

`auto` returns control to the real weather.

## Brightness entity

The Home Assistant brightness entity uses:

``` text
minimum: 0
maximum: 100
step: 10
unit: %
```

The Jar publishes its current brightness percentage to:

``` text
weatherjar/state/brightness
```

The touchscreen no longer needs Home Assistant to control brightness: it
uses the Jar's HTTP API directly. Home Assistant and the touchscreen
therefore act as two independent control surfaces for the same Jar
state.

## Home Assistant / Mosquitto setup

A typical Home Assistant OS setup uses the Mosquitto broker add-on.

High-level setup:

1.  Install and start the **Mosquitto broker** add-on in Home Assistant.
2.  Configure MQTT credentials for the Jar.
3.  Put the broker address, port, username and password in the Jar's
    private configuration/secrets.
4.  Reboot or reconnect the Jar.
5.  Confirm that the Jar connects to MQTT and publishes its discovery
    messages.
6.  Open Home Assistant's MQTT integration.
7.  The **Weather Jar** device and its discovered entities should appear
    automatically.

Do not commit MQTT usernames/passwords or Wi-Fi credentials to a public
repository.

------------------------------------------------------------------------

# Configuration and Secrets

Typical Arduino project structure:

``` text
mini_weather_central/
├── mini_weather_central.ino
├── config.h
├── secrets.h
└── types.h
```

## `mini_weather_central.ino`

Main application containing:

-   Wi-Fi setup
-   NTP/time synchronisation
-   Open-Meteo requests
-   current weather parsing
-   five-day forecast cache
-   weather mapping
-   LED calculations
-   sunrise/sunset effects
-   HTTP API
-   mock system
-   MQTT
-   Home Assistant MQTT Discovery
-   main loop

## `config.h`

Contains hardware and behavioural configuration such as:

``` cpp
LED_RING_PIN
LED_RING_COUNT

ENABLE_WEATHER_LIGHT
ENABLE_SOLAR_EFFECTS

WEATHER_UPDATE_INTERVAL
LED_RING_UPDATE_INTERVAL
SOLAR_EFFECT_HALF_WINDOW_SECONDS
```

Settings for the future wet version may also exist in the
firmware/configuration, but they are not required by the current dry
hardware.

## `secrets.h`

Contains private/local configuration such as:

``` cpp
WIFI_SSID
WIFI_PASSWORD

LATITUDE
LONGITUDE

MQTT_BROKER
MQTT_PORT
MQTT_USER
MQTT_PASSWORD
```

Keep this file private.

## Location

The firmware also exposes a human-readable location name, currently:

``` text
London
```

through `/api/status` and `/api/config`.

Latitude and longitude are used for the actual Open-Meteo request. They
do not need to identify an exact home address; approximate local
coordinates are sufficient.

------------------------------------------------------------------------

# Update Intervals

There are two separate update layers.

## Weather Jar → Open-Meteo

The Jar refreshes weather according to:

``` cpp
WEATHER_UPDATE_INTERVAL
```

The current implementation retrieves current weather and the five-day
daily forecast as part of its weather data update.

## Touchscreen → Weather Jar

The ESPHome panel currently polls:

``` text
/api/status    every 30 seconds
/api/forecast  every 15 minutes
```

The panel therefore refreshes its display independently of when the Jar
last contacted Open-Meteo.

------------------------------------------------------------------------

# Libraries

Arduino/ESP32 firmware uses:

``` text
WiFi
WebServer
HTTPClient
ArduinoJson
FastLED
time
PubSubClient
```

External libraries should be installed as required through Arduino
Library Manager.

------------------------------------------------------------------------

# Serial Monitor

Recommended baud rate:

``` text
115200
```

Useful startup information includes:

-   Wi-Fi connection
-   assigned IP address
-   NTP synchronisation
-   Open-Meteo fetch
-   parsed current weather
-   five-day forecast
-   HTTP server startup
-   MQTT broker connection
-   MQTT subscriptions
-   Home Assistant discovery publication

------------------------------------------------------------------------

# Current Feature Set

``` text
Open-Meteo
    │
    ▼
ESP32 Weather Jar
    │
    ├── current weather
    ├── five-day forecast
    ├── sunrise / sunset
    ├── night mode
    ├── user brightness
    ├── mock/test modes
    ├── HTTP API
    ├── MQTT
    ├── Home Assistant discovery
    └── WS2812B LED ring

Local network
    │
    └── ESPHome touchscreen panel
```

------------------------------------------------------------------------

# Future Improvements

The current documented build deliberately stops at the dry LED-ring
version.

A future **wet version** may add:

-   water pump for physical rain
-   ultrasonic mist maker for fog/cloud effects
-   one or more dedicated lightning LEDs
-   precipitation intensity mapping to physical rain
-   intermittent mist based on weather conditions
-   lightning frequency/intensity based on storm severity

Other possible software improvements:

-   OTA firmware updates
-   persistent runtime configuration
-   smoother transitions between changing weather states
-   additional Home Assistant entities/telemetry

These are roadmap items and are **not required for the current Weather
Jar build**.
