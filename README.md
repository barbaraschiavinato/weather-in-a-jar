# Weather Jar

An ESP32-based ambient weather display that represents real weather
conditions through a **WS2812B LED ring**, with automatic sunrise/sunset
scenes, night mode, a local HTTP API, MQTT/Home Assistant integration,
and an optional ESPHome touchscreen control panel.

The current documented build is the **dry version**: it uses the LED
ring only. A future "wet" version may add physical rain, mist and
dedicated lightning LEDs.

------------------------------------------------------------------------

## Current System

``` text
                     Open-Meteo
                         │
                         ▼
                  ┌─────────────┐
                  │ Weather Jar │
                  │    ESP32    │
                  └──────┬──────┘
                         │
                 GPIO 18 │
                         ▼
                  WS2812B Ring
                         │
              ┌──────────┴──────────┐
              │                     │
           HTTP API               MQTT
              │                     │
              ▼                     ▼
      ESPHome Touch Panel     Home Assistant
```

The **Weather Jar ESP32 is the source of truth** for current weather and
forecast data.

The touchscreen does not contact Open-Meteo directly. It reads the Jar's
local HTTP API.

------------------------------------------------------------------------

# Shopping List

The following parts are used for the current build. The touchscreen and its
battery are optional.

| Component | Version / Notes | Link |
|---|---|---|
| 4 Way MOS Switch Module On-Board ESP32-32E | **4 ways version** | https://www.aliexpress.com/item/1005009534991345.html |
| DC5V WS2812B Round Pixel Ring RGB Full Color | **White, 35 LEDs version** | https://www.aliexpress.com/item/1005007342218529.html |
| Guition ESP32-S3-N16R8 JC8048W550 | **Optional touchscreen** | https://www.aliexpress.com/item/1005006715794302.html |
| JST 1.25 mm Plug Battery 3.7V 3000mAh | **Optional, for touchscreen** | https://www.aliexpress.com/item/1005005626755539.html |

------------------------------------------------------------------------

# 3D-Printed Parts

The 3D-printable parts for the main Weather in a Box enclosure are available
on MakerWorld:

**Weather in a Box – ESP32 Real-Time Weather Display**

https://makerworld.com/en/models/3246919-weather-in-a-box-esp32-real-time-weather-display

The optional 5-inch Guition touchscreen uses the following enclosure/base:

**XTouch Pro Official Base for Guition 5-inch LCD**

https://makerworld.com/en/models/1016156-xtouch-pro-official-base-for-guition-5inch-lcd

------------------------------------------------------------------------

# Hardware

## Weather Jar --- Dry Version

### Components

-   **1x ESP32 Quad MOS Board (ESP32-32E, 4-channel MOSFET version with screw terminals)**
-   **1x WS2812B RGB LED ring --- 35 LEDs**
-   **1x suitable 5 V power supply**
-   Hookup/jumper wires
-   Glass jar / lantern or other enclosure

The controller used in the current build is the **ESP32 Quad MOS Board based on the ESP32-32E**, with **four integrated N-channel MOSFET output channels and screw terminals**. In the dry version, the MOSFET channels are not needed to drive the WS2812B ring; the board is used as the main ESP32 controller and as the power/wiring hub.

No USB-C splitter is used in the current build.

## Power and Wiring

The power supply is connected directly to the ESP32 power screw
terminals.

The LED ring takes its 5 V and GND from the same power connection.

``` text
5 V POWER SUPPLY
      │
      ├──────────────► ESP32 power terminal +5 V
      │                         │
      │                         └────────► WS2812B 5 V
      │
      └──────────────► ESP32 power terminal GND
                                │
                                └────────► WS2812B GND

ESP32 GPIO 18 ───────────────────────────► WS2812B DIN
```

### Pin Mapping

  Device        Connection
  ------------- --------------------------
  WS2812B DIN   GPIO 18
  WS2812B 5 V   ESP32 5 V power terminal
  WS2812B GND   ESP32/common GND
  LED count     35

The ESP32 and ring therefore share the same supply and ground, while
GPIO 18 carries only the WS2812B data signal.

------------------------------------------------------------------------

# Optional Touchscreen Panel

The Weather Jar can be controlled from a separate ESPHome touchscreen.

Current panel hardware:

-   **Guition / Sunton JC8048W550 / JC8048W550C_I**
-   ESP32-S3
-   5-inch 800×480 RGB display
-   GT911 capacitive touchscreen
-   16 MB flash
-   Octal PSRAM
-   2.4 GHz Wi-Fi

The panel uses ESPHome with the ESP-IDF framework.

## Panel Functions

The current UI provides:

-   current live weather
-   current temperature
-   precipitation and rain
-   location
-   five-day forecast
-   forecast min/max temperatures
-   selectable forecast days
-   weather mock controls
-   sunrise and sunset test modes
-   user brightness display
-   brightness `+` / `-` controls
-   loading/activity spinner

## LIVE vs TODAY

### LIVE

`LIVE` displays the actual current state returned by:

``` http
GET /api/status
```

This includes:

-   current weather
-   current temperature
-   current day period
-   precipitation
-   rain
-   location
-   user brightness

### TODAY

`TODAY` represents the **forecast for today**, not the current
observation.

It uses forecast offset `0` and displays:

-   forecast condition
-   maximum temperature
-   minimum temperature
-   `TODAY • FORECAST`

The following four cards display forecast days 1--4.

------------------------------------------------------------------------

# ESPHome Configuration

The complete ESPHome configuration for the optional touchscreen is included
in this repository as:

``` text
weather-panel.yaml
```

This file contains the configuration for the **Guition / Sunton JC8048W550
5-inch 800×480 ESP32-S3 touchscreen**, including the display, GT911 touch
controller, Weather Jar HTTP API integration, live weather view, five-day
forecast, brightness controls and weather/mock controls.

To use it, copy `weather-panel.yaml` into your ESPHome configuration directory
and provide the required values in `secrets.yaml` as described below.

The panel YAML uses secrets instead of embedding local network
information directly in the configuration.

The current YAML expects:

``` yaml
# secrets.yaml

wifi_ssid: "YOUR_2.4_GHZ_WIFI_SSID"
wifi_password: "YOUR_WIFI_PASSWORD"

weather_jar_ip: "192.168.1.xxx"

weather_panel_api_key: "YOUR_ESPHOME_API_ENCRYPTION_KEY"
```

The panel configuration references them as:

``` yaml
substitutions:
  jar_host: !secret weather_jar_ip

api:
  encryption:
    key: !secret weather_panel_api_key

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
```

OTA is enabled through ESPHome:

``` yaml
ota:
  - platform: esphome
```

The current configuration does not require an OTA password secret.

Do not commit a real `secrets.yaml` file to a public repository.

------------------------------------------------------------------------

# Weather Source

Weather information is retrieved by the Jar from **Open-Meteo**.

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

The Jar keeps a **five-day forecast**.

Latitude and longitude determine the weather location, while a
human-readable location name is also exposed to clients such as the
touchscreen.

------------------------------------------------------------------------

# Weather Representation

Open-Meteo weather codes are mapped to internal weather states:

  Weather         Internal value
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

The selected state controls the LED-ring colour and effect brightness.

Typical colours:

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

The Jar uses sunrise and sunset data from Open-Meteo.

The day is divided into four periods:

``` text
night
sunrise
day
sunset
```

Solar transition duration is controlled by:

``` cpp
SOLAR_EFFECT_HALF_WINDOW_SECONDS
```

The complete transition lasts:

``` text
SOLAR_EFFECT_HALF_WINDOW_SECONDS × 2
```

For example:

``` cpp
SOLAR_EFFECT_HALF_WINDOW_SECONDS = 1800;
```

creates a one-hour transition centred on sunrise or sunset.

## Solar Visibility

The solar effect changes according to cloud coverage.

### Full

Used for:

-   clear
-   mainly clear

The ring uses the complete sunrise/sunset colour sequence.

### Partial

Used for:

-   partly cloudy

The solar colours are softer and more muted.

### None

Used for:

-   overcast
-   fog
-   drizzle
-   rain
-   snow
-   thunderstorm

Instead of showing strong sunrise/sunset colours, the normal weather
colour fades in or out.

## Night Mode

At night the LED ring is switched off.

------------------------------------------------------------------------

# Brightness

User brightness is independent from the instantaneous brightness
generated by a weather effect.

The user setting is:

``` text
0–100 %
```

For example:

``` json
"brightnessPercent": 60
```

The status response may simultaneously contain:

``` json
"ring": {
  "brightness": 110,
  "userBrightnessPercent": 60
}
```

This is expected:

-   `brightnessPercent` / `userBrightnessPercent` = user-selected
    brightness
-   `ring.brightness` = instantaneous effect brightness

The touchscreen reads and controls the user brightness directly through
the Jar HTTP API.

------------------------------------------------------------------------

# HTTP API

Once connected to Wi-Fi, the Jar exposes a local HTTP API.

Example:

``` text
http://192.168.1.xxx
```

Use the actual IP address of the Jar.

## Status

``` http
GET /api/status
```

Example:

``` json
{
  "wifi": "connected",
  "ip": "192.168.1.xxx",
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

`source` is normally `real` and becomes `mock` while a mock is active.

## Forecast

``` http
GET /api/forecast
```

Returns five daily forecast entries.

Example:

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

`offset: 0` represents TODAY.

## Configuration

``` http
GET /api/config
```

Returns the current runtime/hardware configuration and location
information.

## Brightness API

``` http
GET /api/brightness
GET /api/brightness?action=up
GET /api/brightness?action=down
GET /api/brightness?value=60
```

`up` and `down` change the user brightness in 10% steps.

------------------------------------------------------------------------

# Mock API

The mock API allows visual states to be tested without waiting for
matching real weather.

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

## Day Period

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

## Accelerated Sunrise/Sunset

``` http
GET /api/mock?weather=clear&period=sunrise&speed=60
GET /api/mock?weather=clear&period=sunset&speed=60
```

## Temporary Mock

``` http
GET /api/mock?weather=rain&duration=60
```

## Return to Live Weather

``` http
GET /api/mock/off
```

------------------------------------------------------------------------

# MQTT and Home Assistant

The Jar connects to an MQTT broker and uses **Home Assistant MQTT
Discovery**.

This allows Home Assistant to discover the Weather Jar and its controls
automatically.

A typical Home Assistant OS installation can use the **Mosquitto broker
add-on**.

## MQTT Topics

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

### Forecast Override

``` text
Command: weatherjar/set/forecast
State:   weatherjar/state/forecast
```

### Availability

``` text
weatherjar/status/availability
```

------------------------------------------------------------------------

# Home Assistant MQTT Discovery

The Jar publishes discovery configuration to:

``` text
homeassistant/select/weatherjar_mode/config
homeassistant/number/weatherjar_brightness/config
homeassistant/binary_sensor/weatherjar_online/config
```

The discovered device is:

``` text
Name:         Weather Jar
Manufacturer: DIY
Model:        ESP32 Weather Jar
```

Home Assistant exposes:

-   **Mode** --- select
-   **Brightness** --- number/slider
-   **Online** --- connectivity binary sensor

## Mode

Available modes are:

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

`auto` returns the Jar to real weather.

## Brightness

The discovered brightness entity uses:

``` text
minimum: 0
maximum: 100
step: 10
unit: %
```

Home Assistant publishes changes to:

``` text
weatherjar/set/brightness
```

and receives the current value from:

``` text
weatherjar/state/brightness
```

The touchscreen does **not** depend on this Home Assistant entity. It
controls brightness directly through `/api/brightness`, so Home
Assistant and the touchscreen are independent control surfaces for the
same Jar.

------------------------------------------------------------------------

# Home Assistant / Mosquitto Setup

Typical setup:

1.  Install the **Mosquitto broker** add-on in Home Assistant.
2.  Start the broker.
3.  Configure the MQTT credentials used by the Weather Jar.
4.  Configure the broker IP/hostname and port in the Jar's private
    configuration.
5.  Flash/restart the Jar.
6.  Confirm that the Jar connects to the MQTT broker.
7.  Confirm that the MQTT integration is enabled in Home Assistant.
8.  The **Weather Jar** device and MQTT Discovery entities should appear
    automatically.

The firmware uses MQTT credentials/configuration such as:

``` cpp
MQTT_BROKER
MQTT_PORT
MQTT_USER
MQTT_PASSWORD
```

Do not commit real MQTT credentials to a public repository.

------------------------------------------------------------------------

# Arduino Project Files

Typical structure:

``` text
weather-in-a-jar/
├── mini_weather_central.ino
├── config.h
├── secrets.example.h
├── types.h
├── weather_colors.h
└── weather-panel.yaml
```

## `mini_weather_central.ino`

Main application containing:

-   Wi-Fi connection
-   NTP/time synchronisation
-   Open-Meteo requests
-   current-weather parsing
-   five-day forecast
-   weather mapping
-   WS2812 effects
-   sunrise/sunset effects
-   night mode
-   HTTP API
-   mock system
-   brightness control
-   MQTT
-   Home Assistant MQTT Discovery
-   main loop

## `config.h`

Contains hardware and behavioural settings such as:

``` cpp
LED_RING_PIN
LED_RING_COUNT

ENABLE_WEATHER_LIGHT
ENABLE_SOLAR_EFFECTS

WEATHER_UPDATE_INTERVAL
LED_RING_UPDATE_INTERVAL
SOLAR_EFFECT_HALF_WINDOW_SECONDS
```

The current dry build uses:

``` text
LED_RING_PIN   = GPIO 18
LED_RING_COUNT = 35
```

The codebase may contain configuration for future hardware, but that
hardware is not required for this build.

## `secrets.h`

Private/local settings include Wi-Fi, location and MQTT connection
details.

Typical values include:

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

Keep real credentials out of a public repository.

------------------------------------------------------------------------

# Update Behaviour

There are two separate update layers.

## Jar → Open-Meteo

The Jar refreshes weather according to:

``` cpp
WEATHER_UPDATE_INTERVAL
```

The Jar stores the current weather and daily forecast locally.

## Touchscreen → Jar

The ESPHome panel reads:

``` text
/api/status    every 30 seconds
/api/forecast  every 15 minutes
```

At startup, the panel waits for the display and Wi-Fi to settle, then
loads the data sequentially:

``` text
UI initialisation
      ↓
Wi-Fi connected
      ↓
/api/status
      ↓
/api/forecast
      ↓
normal periodic refresh
```

This avoids overlapping startup requests and reduces display instability
during initial connection.

------------------------------------------------------------------------

# Libraries

The Arduino firmware uses:

``` text
WiFi
WebServer
HTTPClient
ArduinoJson
FastLED
time
PubSubClient
```

External libraries should be installed through Arduino Library Manager
where required.

------------------------------------------------------------------------

# Serial Monitor

Recommended baud rate:

``` text
115200
```

Useful startup diagnostics include:

-   Wi-Fi connection
-   assigned IP address
-   NTP synchronisation
-   Open-Meteo request
-   current weather
-   five-day forecast
-   HTTP server
-   MQTT connection
-   MQTT subscriptions
-   Home Assistant Discovery

------------------------------------------------------------------------

# Current Feature Set

``` text
Open-Meteo
    │
    ▼
Weather Jar ESP32
    │
    ├── current weather
    ├── five-day forecast
    ├── sunrise / sunset
    ├── night mode
    ├── WS2812B ring
    ├── user brightness
    ├── mock/test modes
    ├── HTTP API
    ├── MQTT
    └── Home Assistant Discovery
          │
          └── Home Assistant

Weather Jar HTTP API
    │
    └── ESPHome touchscreen
```

------------------------------------------------------------------------

# Future Improvements

The current documented hardware is intentionally limited to the dry
LED-ring version.

A future **wet version** may add:

-   **1x Mini Submersible Water Pump (5V DC):** used to physically
    simulate rain inside the jar.
-   **1x Mini Ultrasonic Mist Maker (5V DC):** used to generate
    atmospheric fog/mist based on real-time weather conditions.
-   **2x Standard 5V LEDs (White/Cool White):** dedicated high-intensity
    light sources used to simulate quick lightning flashes during
    thunderstorms.

These components could enable:

-   physical rain simulation
-   precipitation intensity mapping
-   intermittent mist for cloud/fog conditions
-   dedicated lightning flashes
-   lightning frequency/intensity based on storm severity

Other possible software improvements:

-   OTA workflow improvements
-   persistent runtime configuration
-   smoother transitions between changing weather states
-   additional Home Assistant entities and telemetry

The pump, mist maker and dedicated lightning LEDs are **future hardware
and are not required for the current dry build**.
