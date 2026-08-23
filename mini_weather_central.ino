#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <time.h>

#include "config.h"
#include "secrets.h"
#include "types.h"

// ============================================================
// HARDWARE
// ============================================================

CRGB leds[LED_RING_COUNT];

WebServer server(80);

// ============================================================
// GLOBAL STATE
// ============================================================

WeatherState weatherState;
MockState mockState;
LedState ledState;
LightningState lightningState;

unsigned long lastWeatherUpdate = 0;
unsigned long lastLedUpdate = 0;

// ============================================================
// COLORS
// ============================================================

constexpr RgbColor COLOR_OFF = {
  0, 0, 0
};

constexpr RgbColor COLOR_WARM_WHITE = {
  255, 190, 120
};

constexpr RgbColor COLOR_COOL_WHITE = {
  200, 220, 255
};

constexpr RgbColor COLOR_OVERCAST = {
  95, 110, 125
};

constexpr RgbColor COLOR_RAIN = {
  35, 90, 180
};

constexpr RgbColor COLOR_FOG = {
  150, 165, 170
};

constexpr RgbColor COLOR_SNOW = {
  210, 225, 255
};

constexpr RgbColor COLOR_STORM = {
  25, 35, 80
};

constexpr RgbColor COLOR_DEEP_RED = {
  150, 10, 0
};

constexpr RgbColor COLOR_RED = {
  230, 35, 5
};

constexpr RgbColor COLOR_ORANGE = {
  255, 90, 10
};

constexpr RgbColor COLOR_AMBER = {
  255, 155, 35
};

// ============================================================
// COLOR HELPERS
// ============================================================

RgbColor interpolateColor(
  const RgbColor& a,
  const RgbColor& b,
  float t
) {
  t = constrain(
    t,
    0.0f,
    1.0f
  );

  RgbColor result;

  result.r = (uint8_t)(
    a.r +
    (b.r - a.r) * t
  );

  result.g = (uint8_t)(
    a.g +
    (b.g - a.g) * t
  );

  result.b = (uint8_t)(
    a.b +
    (b.b - a.b) * t
  );

  return result;
}

float smoothStep(float t) {
  t = constrain(
    t,
    0.0f,
    1.0f
  );

  return
    t *
    t *
    (3.0f - 2.0f * t);
}

RgbColor interpolateMultiColor(
  const RgbColor colors[],
  int colorCount,
  float progress
) {
  progress = constrain(
    progress,
    0.0f,
    1.0f
  );

  if (colorCount <= 1) {
    return colors[0];
  }

  float scaled =
    progress *
    (colorCount - 1);

  int index =
    floor(scaled);

  float localProgress =
    scaled - index;

  if (
    index >=
    colorCount - 1
  ) {
    return colors[
      colorCount - 1
    ];
  }

  return interpolateColor(
    colors[index],
    colors[index + 1],
    smoothStep(localProgress)
  );
}

// ============================================================
// STRING HELPERS
// ============================================================

String weatherToString(
  WeatherType weather
) {
  switch (weather) {
    case WeatherType::CLEAR:
      return "clear";

    case WeatherType::MAINLY_CLEAR:
      return "mainly_clear";

    case WeatherType::PARTLY_CLOUDY:
      return "partly_cloudy";

    case WeatherType::OVERCAST:
      return "overcast";

    case WeatherType::FOG:
      return "fog";

    case WeatherType::DRIZZLE:
      return "drizzle";

    case WeatherType::RAIN:
      return "rain";

    case WeatherType::SNOW:
      return "snow";

    case WeatherType::THUNDERSTORM:
      return "thunderstorm";

    default:
      return "unknown";
  }
}

String periodToString(
  DayPeriod period
) {
  switch (period) {
    case DayPeriod::NIGHT:
      return "night";

    case DayPeriod::SUNRISE:
      return "sunrise";

    case DayPeriod::DAY:
      return "day";

    case DayPeriod::SUNSET:
      return "sunset";
  }

  return "unknown";
}

String visibilityToString(
  SunVisibility visibility
) {
  switch (visibility) {
    case SunVisibility::FULL:
      return "full";

    case SunVisibility::PARTIAL:
      return "partial";

    case SunVisibility::NONE:
      return "none";
  }

  return "none";
}

// ============================================================
// PARSERS
// ============================================================

bool parseWeather(
  const String& value,
  WeatherType& result
) {
  if (value == "clear") {
    result = WeatherType::CLEAR;
    return true;
  }

  if (value == "mainly_clear") {
    result = WeatherType::MAINLY_CLEAR;
    return true;
  }

  if (value == "partly_cloudy") {
    result = WeatherType::PARTLY_CLOUDY;
    return true;
  }

  if (value == "overcast") {
    result = WeatherType::OVERCAST;
    return true;
  }

  if (value == "fog") {
    result = WeatherType::FOG;
    return true;
  }

  if (value == "drizzle") {
    result = WeatherType::DRIZZLE;
    return true;
  }

  if (value == "rain") {
    result = WeatherType::RAIN;
    return true;
  }

  if (value == "snow") {
    result = WeatherType::SNOW;
    return true;
  }

  if (value == "thunderstorm") {
    result = WeatherType::THUNDERSTORM;
    return true;
  }

  return false;
}

bool parsePeriod(
  const String& value,
  DayPeriod& result
) {
  if (value == "night") {
    result = DayPeriod::NIGHT;
    return true;
  }

  if (value == "sunrise") {
    result = DayPeriod::SUNRISE;
    return true;
  }

  if (value == "day") {
    result = DayPeriod::DAY;
    return true;
  }

  if (value == "sunset") {
    result = DayPeriod::SUNSET;
    return true;
  }

  return false;
}

// ============================================================
// OPEN-METEO WEATHER CODE
// ============================================================

WeatherType weatherCodeToType(
  int code
) {
  if (code == 0) {
    return WeatherType::CLEAR;
  }

  if (code == 1) {
    return WeatherType::MAINLY_CLEAR;
  }

  if (code == 2) {
    return WeatherType::PARTLY_CLOUDY;
  }

  if (code == 3) {
    return WeatherType::OVERCAST;
  }

  if (
    code == 45 ||
    code == 48
  ) {
    return WeatherType::FOG;
  }

  if (
    code >= 51 &&
    code <= 57
  ) {
    return WeatherType::DRIZZLE;
  }

  if (
    (code >= 61 && code <= 67) ||
    (code >= 80 && code <= 82)
  ) {
    return WeatherType::RAIN;
  }

  if (
    (code >= 71 && code <= 77) ||
    (code >= 85 && code <= 86)
  ) {
    return WeatherType::SNOW;
  }

  if (
    code >= 95 &&
    code <= 99
  ) {
    return WeatherType::THUNDERSTORM;
  }

  return WeatherType::UNKNOWN;
}

// ============================================================
// WEATHER COLORS
// ============================================================

RgbColor getWeatherColor(
  WeatherType weather
) {
  switch (weather) {
    case WeatherType::CLEAR:
      return COLOR_WARM_WHITE;

    case WeatherType::MAINLY_CLEAR:
      return {
        245, 205, 150
      };

    case WeatherType::PARTLY_CLOUDY:
      return {
        220, 220, 205
      };

    case WeatherType::OVERCAST:
      return COLOR_OVERCAST;

    case WeatherType::FOG:
      return COLOR_FOG;

    case WeatherType::DRIZZLE:
      return {
        70, 125, 180
      };

    case WeatherType::RAIN:
      return COLOR_RAIN;

    case WeatherType::SNOW:
      return COLOR_SNOW;

    case WeatherType::THUNDERSTORM:
      return COLOR_STORM;

    default:
      return COLOR_COOL_WHITE;
  }
}

uint8_t getWeatherBrightness(
  WeatherType weather
) {
  switch (weather) {
    case WeatherType::CLEAR:
      return BRIGHTNESS_CLEAR;

    case WeatherType::MAINLY_CLEAR:
      return BRIGHTNESS_MAINLY_CLEAR;

    case WeatherType::PARTLY_CLOUDY:
      return BRIGHTNESS_PARTLY_CLOUDY;

    case WeatherType::OVERCAST:
      return BRIGHTNESS_OVERCAST;

    case WeatherType::FOG:
      return BRIGHTNESS_FOG;

    case WeatherType::DRIZZLE:
      return BRIGHTNESS_DRIZZLE;

    case WeatherType::RAIN:
      return BRIGHTNESS_RAIN;

    case WeatherType::SNOW:
      return BRIGHTNESS_SNOW;

    case WeatherType::THUNDERSTORM:
      return BRIGHTNESS_THUNDERSTORM;

    default:
      return BRIGHTNESS_UNKNOWN;
  }
}

// ============================================================
// SUN VISIBILITY
// ============================================================

SunVisibility getSunVisibility(
  WeatherType weather
) {
  switch (weather) {
    case WeatherType::CLEAR:
    case WeatherType::MAINLY_CLEAR:
      return SunVisibility::FULL;

    case WeatherType::PARTLY_CLOUDY:
      return SunVisibility::PARTIAL;

    default:
      return SunVisibility::NONE;
  }
}

// ============================================================
// EFFECTIVE WEATHER
// ============================================================

WeatherType getEffectiveWeather() {
  if (
    mockState.enabled &&
    mockState.overrideWeather
  ) {
    return mockState.weather;
  }

  return weatherState.weather;
}

// ============================================================
// DAY PERIOD
// ============================================================

DayPeriod getRealPeriod() {
  if (!weatherState.valid) {
    return DayPeriod::DAY;
  }

  time_t now =
    time(nullptr);

  time_t sunriseStart =
    weatherState.sunrise -
    SOLAR_EFFECT_HALF_WINDOW_SECONDS;

  time_t sunriseEnd =
    weatherState.sunrise +
    SOLAR_EFFECT_HALF_WINDOW_SECONDS;

  time_t sunsetStart =
    weatherState.sunset -
    SOLAR_EFFECT_HALF_WINDOW_SECONDS;

  time_t sunsetEnd =
    weatherState.sunset +
    SOLAR_EFFECT_HALF_WINDOW_SECONDS;

  if (now < sunriseStart) {
    return DayPeriod::NIGHT;
  }

  if (now <= sunriseEnd) {
    return DayPeriod::SUNRISE;
  }

  if (now < sunsetStart) {
    return DayPeriod::DAY;
  }

  if (now <= sunsetEnd) {
    return DayPeriod::SUNSET;
  }

  return DayPeriod::NIGHT;
}

DayPeriod getEffectivePeriod() {
  if (
    mockState.enabled &&
    mockState.overridePeriod
  ) {
    return mockState.period;
  }

  return getRealPeriod();
}

// ============================================================
// SOLAR PROGRESS
// ============================================================

float getRealSolarProgress(
  DayPeriod period
) {
  time_t now =
    time(nullptr);

  if (
    period ==
    DayPeriod::SUNRISE
  ) {
    time_t start =
      weatherState.sunrise -
      SOLAR_EFFECT_HALF_WINDOW_SECONDS;

    time_t end =
      weatherState.sunrise +
      SOLAR_EFFECT_HALF_WINDOW_SECONDS;

    return constrain(
      (float)(now - start) /
      (float)(end - start),
      0.0f,
      1.0f
    );
  }

  if (
    period ==
    DayPeriod::SUNSET
  ) {
    time_t start =
      weatherState.sunset -
      SOLAR_EFFECT_HALF_WINDOW_SECONDS;

    time_t end =
      weatherState.sunset +
      SOLAR_EFFECT_HALF_WINDOW_SECONDS;

    return constrain(
      (float)(now - start) /
      (float)(end - start),
      0.0f,
      1.0f
    );
  }

  return 0.0f;
}

float getMockSolarProgress() {
  float duration =
    3600.0f /
    mockState.speed;

  if (duration <= 0.0f) {
    return 1.0f;
  }

  float elapsed =
    (millis() -
     mockState.startedAt) /
    1000.0f;

  return constrain(
    elapsed / duration,
    0.0f,
    1.0f
  );
}

float getSolarProgress(
  DayPeriod period
) {
  if (
    mockState.enabled &&
    mockState.overridePeriod &&
    (
      period == DayPeriod::SUNRISE ||
      period == DayPeriod::SUNSET
    )
  ) {
    return getMockSolarProgress();
  }

  return getRealSolarProgress(
    period
  );
}

// ============================================================
// SUNRISE
// ============================================================

LedState calculateSunrise(
  WeatherType weather,
  SunVisibility visibility,
  float progress
) {
  LedState result;

  RgbColor weatherColor =
    getWeatherColor(weather);

  uint8_t weatherBrightness =
    getWeatherBrightness(weather);

  progress =
    smoothStep(progress);

  if (
    visibility ==
    SunVisibility::FULL
  ) {
    const RgbColor colors[] = {
      COLOR_OFF,
      COLOR_DEEP_RED,
      COLOR_RED,
      COLOR_ORANGE,
      COLOR_AMBER,
      COLOR_WARM_WHITE,
      weatherColor
    };

    result.color =
      interpolateMultiColor(
        colors,
        7,
        progress
      );

    result.brightness =
      (uint8_t)(
        10 +
        (weatherBrightness - 10) *
        progress
      );

    result.effect =
      "sunrise";

  } else if (
    visibility ==
    SunVisibility::PARTIAL
  ) {
    const RgbColor colors[] = {
      COLOR_OFF,
      {120, 35, 15},
      {210, 95, 40},
      {230, 175, 120},
      weatherColor
    };

    result.color =
      interpolateMultiColor(
        colors,
        5,
        progress
      );

    result.brightness =
      (uint8_t)(
        5 +
        (weatherBrightness - 5) *
        progress
      );

    result.effect =
      "sunrise_partial";

  } else {
    result.color =
      weatherColor;

    result.brightness =
      (uint8_t)(
        weatherBrightness *
        progress
      );

    result.effect =
      "fade_in";
  }

  result.progress =
    progress;

  return result;
}

// ============================================================
// SUNSET
// ============================================================

LedState calculateSunset(
  WeatherType weather,
  SunVisibility visibility,
  float progress
) {
  LedState result;

  RgbColor weatherColor =
    getWeatherColor(weather);

  uint8_t weatherBrightness =
    getWeatherBrightness(weather);

  progress =
    smoothStep(progress);

  if (
    visibility ==
    SunVisibility::FULL
  ) {
    const RgbColor colors[] = {
      weatherColor,
      COLOR_WARM_WHITE,
      COLOR_AMBER,
      COLOR_ORANGE,
      COLOR_RED,
      COLOR_DEEP_RED,
      COLOR_OFF
    };

    result.color =
      interpolateMultiColor(
        colors,
        7,
        progress
      );

    result.brightness =
      (uint8_t)(
        weatherBrightness *
        (1.0f - progress)
      );

    result.effect =
      "sunset";

  } else if (
    visibility ==
    SunVisibility::PARTIAL
  ) {
    const RgbColor colors[] = {
      weatherColor,
      {230, 175, 120},
      {210, 95, 40},
      {120, 35, 15},
      COLOR_OFF
    };

    result.color =
      interpolateMultiColor(
        colors,
        5,
        progress
      );

    result.brightness =
      (uint8_t)(
        weatherBrightness *
        (1.0f - progress)
      );

    result.effect =
      "sunset_partial";

  } else {
    result.color =
      weatherColor;

    result.brightness =
      (uint8_t)(
        weatherBrightness *
        (1.0f - progress)
      );

    result.effect =
      "fade_out";
  }

  result.progress =
    progress;

  return result;
}

// ============================================================
// LED STATE CALCULATION
// ============================================================

LedState calculateLedState() {
  LedState result;

  if (!ENABLE_WEATHER_LIGHT) {
    result.effect =
      "disabled";

    return result;
  }

  WeatherType weather =
    getEffectiveWeather();

  DayPeriod period =
    getEffectivePeriod();

  SunVisibility visibility =
    getSunVisibility(weather);

  if (
    period ==
    DayPeriod::NIGHT
  ) {
    result.color =
      COLOR_OFF;

    result.brightness =
      0;

    result.effect =
      "night";

    return result;
  }

  if (
    ENABLE_SOLAR_EFFECTS &&
    period ==
    DayPeriod::SUNRISE
  ) {
    return calculateSunrise(
      weather,
      visibility,
      getSolarProgress(period)
    );
  }

  if (
    ENABLE_SOLAR_EFFECTS &&
    period ==
    DayPeriod::SUNSET
  ) {
    return calculateSunset(
      weather,
      visibility,
      getSolarProgress(period)
    );
  }

  result.color =
    getWeatherColor(weather);

  result.brightness =
    getWeatherBrightness(weather);

  result.effect =
    weatherToString(weather);

  result.progress =
    0.0f;

  return result;
}

// ============================================================
// FASTLED OUTPUT
// ============================================================

bool ledOutputChanged(
  const LedState& a,
  const LedState& b
) {
  return
    a.color.r != b.color.r ||
    a.color.g != b.color.g ||
    a.color.b != b.color.b ||
    a.brightness != b.brightness;
}

void applyLedState(
  const LedState& state
) {
  FastLED.setBrightness(
    state.brightness
  );

  CRGB color(
    state.color.r,
    state.color.g,
    state.color.b
  );

  fill_solid(
    leds,
    LED_RING_COUNT,
    color
  );

  FastLED.show();
}

// ============================================================
// LIGHTNING
// ============================================================

void setLightningBrightness(
  uint8_t brightness
) {
  lightningState.brightness =
    brightness;

  if (!ENABLE_LIGHTNING_LED) {
    return;
  }

  ledcWrite(
    LIGHTNING_LED_PIN,
    brightness
  );
}

void setLightningBrightness2(
  uint8_t brightness
) {
  lightningState.brightness2 =
    brightness;

  if (!ENABLE_LIGHTNING_LED_2) {
    return;
  }

  ledcWrite(
    LIGHTNING_LED_2_PIN,
    brightness
  );
}

void stopLightningOutputs() {
  setLightningBrightness(0);
  setLightningBrightness2(0);

  lightningState.led2Pending = false;
  lightningState.led2InFlash = false;
}

void scheduleNextLightningEvent() {
  lightningState.nextEventAt =
    millis() +
    random(
      LIGHTNING_EVENT_MIN_MS,
      LIGHTNING_EVENT_MAX_MS + 1
    );
}

void beginLightningEvent() {
  lightningState.active =
    true;

  lightningState.inFlash =
    false;

  lightningState.flashesRemaining =
    random(
      1,
      LIGHTNING_MAX_FLASHES_PER_EVENT + 1
    );

  lightningState.stateUntil =
    millis();
}

void updateLightningLed2(
  unsigned long now
) {
  if (!ENABLE_LIGHTNING_LED_2) {
    return;
  }

  if (
    lightningState.led2Pending &&
    (long)(now - lightningState.led2StartAt) >= 0
  ) {
    setLightningBrightness2(
      lightningState.led2PendingBrightness
    );

    lightningState.led2Pending = false;
    lightningState.led2InFlash = true;
  }

  if (
    lightningState.led2InFlash &&
    (long)(now - lightningState.led2EndAt) >= 0
  ) {
    setLightningBrightness2(0);
    lightningState.led2InFlash = false;
  }
}

void updateLightning() {
  bool thunderstorm =
    getEffectiveWeather() ==
    WeatherType::THUNDERSTORM;

  if (
    !ENABLE_LIGHTNING_LED ||
    !thunderstorm
  ) {
    lightningState.active = false;
    stopLightningOutputs();
    return;
  }

  unsigned long now =
    millis();

  updateLightningLed2(now);

  if (
    lightningState.nextEventAt == 0
  ) {
    scheduleNextLightningEvent();
  }

  if (
    !lightningState.active &&
    (long)(
      now -
      lightningState.nextEventAt
    ) >= 0
  ) {
    beginLightningEvent();
  }

  if (!lightningState.active) {
    return;
  }

  if (
    (long)(
      now -
      lightningState.stateUntil
    ) < 0
  ) {
    return;
  }

  if (!lightningState.inFlash) {
    uint8_t brightness =
      random(
        LIGHTNING_MIN_BRIGHTNESS,
        LIGHTNING_MAX_BRIGHTNESS + 1
      );

    unsigned long flashDuration =
      random(
        LIGHTNING_FLASH_MIN_MS,
        LIGHTNING_FLASH_MAX_MS + 1
      );

    setLightningBrightness(
      brightness
    );

    lightningState.inFlash =
      true;

    lightningState.stateUntil =
      now + flashDuration;

    if (ENABLE_LIGHTNING_LED_2) {
      lightningState.led2PendingBrightness =
        brightness;
      lightningState.led2StartAt =
        now + LIGHTNING_LED_2_DELAY_MS;
      lightningState.led2EndAt =
        lightningState.led2StartAt + flashDuration;
      lightningState.led2Pending = true;
      lightningState.led2InFlash = false;
    }

  } else {
    setLightningBrightness(0);

    lightningState.inFlash =
      false;

    lightningState.flashesRemaining--;

    if (
      lightningState.flashesRemaining <= 0
    ) {
      lightningState.active =
        false;

      scheduleNextLightningEvent();

    } else {
      // Ensure the delayed second flash has time to finish before
      // starting the next flash in the same lightning event.
      unsigned long led2Extra =
        ENABLE_LIGHTNING_LED_2
          ? LIGHTNING_LED_2_DELAY_MS
          : 0;

      lightningState.stateUntil =
        now +
        led2Extra +
        random(
          LIGHTNING_GAP_MIN_MS,
          LIGHTNING_GAP_MAX_MS + 1
        );
    }
  }
}

// ============================================================
// WEATHER UPDATE
// ============================================================

bool updateWeather() {
  if (
    WiFi.status() !=
    WL_CONNECTED
  ) {
    Serial.println(
      "Weather update skipped: Wi-Fi disconnected."
    );

    return false;
  }

  HTTPClient http;

  String url =
    "https://api.open-meteo.com/v1/forecast"
    "?latitude=" +
    String(LATITUDE, 6) +
    "&longitude=" +
    String(LONGITUDE, 6) +
    "&current=temperature_2m,weather_code,precipitation,rain"
    "&daily=sunrise,sunset"
    "&timezone=auto"
    "&forecast_days=1";

  Serial.println();
  Serial.println(
    "Fetching weather..."
  );

  int httpCode = -1;

  http.begin(url);

  httpCode =
    http.GET();

  if (
    httpCode !=
    HTTP_CODE_OK
  ) {
    Serial.printf(
      "HTTP error: %d\n",
      httpCode
    );

    http.end();

    return false;
  }

  String payload =
    http.getString();

  http.end();

  JsonDocument doc;

  DeserializationError error =
    deserializeJson(
      doc,
      payload
    );

  if (error) {
    Serial.print(
      "JSON error: "
    );

    Serial.println(
      error.c_str()
    );

    return false;
  }

  weatherState.temperature =
    doc["current"]["temperature_2m"]
      | 0.0f;

  weatherState.weatherCode =
    doc["current"]["weather_code"]
      | -1;

  weatherState.precipitation =
    doc["current"]["precipitation"]
      | 0.0f;

  weatherState.rain =
    doc["current"]["rain"]
      | 0.0f;

  weatherState.weather =
    weatherCodeToType(
      weatherState.weatherCode
    );

  String sunriseString =
    doc["daily"]["sunrise"][0]
      | "";

  String sunsetString =
    doc["daily"]["sunset"][0]
      | "";

  struct tm sunriseTm = {};
  struct tm sunsetTm = {};

  if (
    strptime(
      sunriseString.c_str(),
      "%Y-%m-%dT%H:%M",
      &sunriseTm
    )
  ) {
    sunriseTm.tm_isdst = -1;

    weatherState.sunrise =
      mktime(
        &sunriseTm
      );
  }

  if (
    strptime(
      sunsetString.c_str(),
      "%Y-%m-%dT%H:%M",
      &sunsetTm
    )
  ) {
    sunsetTm.tm_isdst = -1;

    weatherState.sunset =
      mktime(
        &sunsetTm
      );
  }

  weatherState.valid =
    true;

  Serial.println();
  Serial.println(
    "=========================="
  );

  Serial.printf(
    "Temperature: %.2f C\n",
    weatherState.temperature
  );

  Serial.printf(
    "Weather code: %d\n",
    weatherState.weatherCode
  );

  Serial.printf(
    "Precipitation: %.2f mm\n",
    weatherState.precipitation
  );

  Serial.printf(
    "Rain: %.2f mm\n",
    weatherState.rain
  );

  Serial.println();

  Serial.print(
    "WEATHER: "
  );

  Serial.println(
    weatherToString(
      weatherState.weather
    )
  );

  Serial.println(
    "=========================="
  );

  return true;
}

// ============================================================
// MOCK EXPIRATION
// ============================================================

void updateMockExpiration() {
  if (!mockState.enabled) {
    return;
  }

  if (
    mockState.expiresAt > 0 &&
    (long)(
      millis() -
      mockState.expiresAt
    ) >= 0
  ) {
    Serial.println(
      "Mock expired. Returning to real mode."
    );

    mockState =
      MockState();

    LedState newState =
      calculateLedState();

    if (
      ledOutputChanged(
        ledState,
        newState
      )
    ) {
      applyLedState(
        newState
      );
    }

    ledState =
      newState;
  }
}

// ============================================================
// API - STATUS
// ============================================================

void handleStatus() {
  WeatherType weather =
    getEffectiveWeather();

  DayPeriod period =
    getEffectivePeriod();

  SunVisibility visibility =
    getSunVisibility(weather);

  JsonDocument doc;

  doc["wifi"] =
    WiFi.status() ==
    WL_CONNECTED
      ? "connected"
      : "disconnected";

  doc["ip"] =
    WiFi.localIP()
      .toString();

  doc["source"] =
    mockState.enabled
      ? "mock"
      : "real";

  doc["weather"] =
    weatherToString(weather);

  doc["period"] =
    periodToString(period);

  doc["sunVisibility"] =
    visibilityToString(
      visibility
    );

  doc["temperature"] =
    weatherState.temperature;

  doc["weatherCode"] =
    weatherState.weatherCode;

  doc["precipitation"] =
    weatherState.precipitation;

  doc["rain"] =
    weatherState.rain;

  JsonObject ring =
    doc["ring"].to<JsonObject>();

  ring["enabled"] =
    ENABLE_WEATHER_LIGHT;

  ring["effect"] =
    ledState.effect;

  ring["progress"] =
    ledState.progress;

  ring["brightness"] =
    ledState.brightness;

  ring["r"] =
    ledState.color.r;

  ring["g"] =
    ledState.color.g;

  ring["b"] =
    ledState.color.b;

  JsonObject lightning =
    doc["lightning"].to<JsonObject>();

  lightning["enabled"] =
    ENABLE_LIGHTNING_LED;

  lightning["active"] =
    lightningState.active;

  lightning["brightness"] =
    lightningState.brightness;

  lightning["led2Enabled"] =
    ENABLE_LIGHTNING_LED_2;

  lightning["brightness2"] =
    lightningState.brightness2;

  String response;

  serializeJsonPretty(
    doc,
    response
  );

  server.send(
    200,
    "application/json",
    response
  );
}

// ============================================================
// API - CONFIG
// ============================================================

void handleConfig() {
  JsonDocument doc;

  doc["latitude"] =
    LATITUDE;

  doc["longitude"] =
    LONGITUDE;

  doc["weatherUpdateSeconds"] =
    WEATHER_UPDATE_INTERVAL /
    1000UL;

  doc["solarEffectWindowMinutes"] =
    SOLAR_EFFECT_HALF_WINDOW_SECONDS *
    2 /
    60;

  JsonObject ring =
    doc["ring"].to<JsonObject>();

  ring["enabled"] =
    ENABLE_WEATHER_LIGHT;

  ring["pin"] =
    LED_RING_PIN;

  ring["count"] =
    LED_RING_COUNT;

  ring["updateIntervalMs"] =
    LED_RING_UPDATE_INTERVAL;

  JsonObject lightning =
    doc["lightning"].to<JsonObject>();

  lightning["enabled"] =
    ENABLE_LIGHTNING_LED;

  lightning["pin"] =
    LIGHTNING_LED_PIN;

  lightning["led2Enabled"] =
    ENABLE_LIGHTNING_LED_2;

  lightning["led2Pin"] =
    LIGHTNING_LED_2_PIN;

  lightning["led2DelayMs"] =
    LIGHTNING_LED_2_DELAY_MS;

  JsonObject pump =
    doc["pump"].to<JsonObject>();

  pump["enabled"] =
    ENABLE_PUMP;

  pump["pin"] =
    PUMP_PIN;

  JsonObject mister =
    doc["mister"].to<JsonObject>();

  mister["enabled"] =
    ENABLE_MISTER;

  mister["pin"] =
    MISTER_PIN;

  String response;

  serializeJsonPretty(
    doc,
    response
  );

  server.send(
    200,
    "application/json",
    response
  );
}

// ============================================================
// API - MOCK
// ============================================================

void handleMock() {
  bool changed = false;
  bool restartAnimation = false;

  if (
    server.hasArg("weather")
  ) {
    WeatherType weather;

    if (
      !parseWeather(
        server.arg("weather"),
        weather
      )
    ) {
      server.send(
        400,
        "application/json",
        "{\"error\":\"Invalid weather\"}"
      );

      return;
    }

    mockState.weather =
      weather;

    mockState.overrideWeather =
      true;

    changed =
      true;
  }

  if (
    server.hasArg("period")
  ) {
    DayPeriod period;

    if (
      !parsePeriod(
        server.arg("period"),
        period
      )
    ) {
      server.send(
        400,
        "application/json",
        "{\"error\":\"Invalid period\"}"
      );

      return;
    }

    mockState.period =
      period;

    mockState.overridePeriod =
      true;

    changed =
      true;

    restartAnimation =
      true;
  }

  if (
    server.hasArg("speed")
  ) {
    float speed =
      server.arg("speed")
        .toFloat();

    if (speed <= 0.0f) {
      server.send(
        400,
        "application/json",
        "{\"error\":\"Speed must be > 0\"}"
      );

      return;
    }

    mockState.speed =
      speed;

    changed =
      true;

    restartAnimation =
      true;
  }

  if (
    server.hasArg("duration")
  ) {
    unsigned long duration =
      server.arg("duration")
        .toInt();

    if (duration > 0) {
      mockState.expiresAt =
        millis() +
        duration *
        1000UL;
    }

    changed =
      true;
  }

  if (!changed) {
    server.send(
      400,
      "application/json",
      "{\"error\":\"No mock parameters supplied\"}"
    );

    return;
  }

  if (
    !mockState.enabled ||
    restartAnimation
  ) {
    mockState.startedAt =
      millis();
  }

  mockState.enabled =
    true;

  // A thunderstorm mock starts a lightning event immediately,
  // so both configured lightning LEDs can be tested without waiting
  // for the normal random event interval.
  if (
    getEffectiveWeather() == WeatherType::THUNDERSTORM &&
    ENABLE_LIGHTNING_LED
  ) {
    lightningState = LightningState();
    beginLightningEvent();
  }

  LedState newState =
    calculateLedState();

  if (
    ledOutputChanged(
      ledState,
      newState
    )
  ) {
    applyLedState(
      newState
    );
  }

  ledState =
    newState;

  handleStatus();
}

// ============================================================
// API - MOCK OFF
// ============================================================

void handleMockOff() {
  mockState =
    MockState();

  lightningState =
    LightningState();

  stopLightningOutputs();

  Serial.println(
    "Mock disabled. Returning to real weather."
  );

  LedState newState =
    calculateLedState();

  if (
    ledOutputChanged(
      ledState,
      newState
    )
  ) {
    applyLedState(
      newState
    );
  }

  ledState =
    newState;

  handleStatus();
}

// ============================================================
// API - ROOT
// ============================================================

void handleRoot() {
  String text =
    "Weather Jar V1\n"
    "\n"
    "STATUS\n"
    "GET /api/status\n"
    "GET /api/config\n"
    "\n"
    "WEATHER MOCKS\n"
    "GET /api/mock?weather=clear\n"
    "GET /api/mock?weather=mainly_clear\n"
    "GET /api/mock?weather=partly_cloudy\n"
    "GET /api/mock?weather=overcast\n"
    "GET /api/mock?weather=fog\n"
    "GET /api/mock?weather=drizzle\n"
    "GET /api/mock?weather=rain\n"
    "GET /api/mock?weather=snow\n"
    "GET /api/mock?weather=thunderstorm\n"
    "\n"
    "PERIOD MOCKS\n"
    "GET /api/mock?period=night\n"
    "GET /api/mock?period=sunrise\n"
    "GET /api/mock?period=day\n"
    "GET /api/mock?period=sunset\n"
    "\n"
    "COMBINED\n"
    "GET /api/mock?weather=clear&period=sunrise\n"
    "GET /api/mock?weather=clear&period=sunset\n"
    "GET /api/mock?weather=partly_cloudy&period=sunrise\n"
    "GET /api/mock?weather=partly_cloudy&period=sunset\n"
    "GET /api/mock?weather=rain&period=sunrise\n"
    "GET /api/mock?weather=rain&period=sunset\n"
    "\n"
    "ACCELERATED SOLAR EFFECT\n"
    "GET /api/mock?weather=clear&period=sunrise&speed=60\n"
    "GET /api/mock?weather=clear&period=sunset&speed=60\n"
    "\n"
    "TEMPORARY MOCK\n"
    "GET /api/mock?weather=thunderstorm&duration=60\n"
    "\n"
    "DISABLE MOCK\n"
    "GET /api/mock/off\n";

  server.send(
    200,
    "text/plain",
    text
  );
}

// ============================================================
// WEB SERVER
// ============================================================

void setupServer() {
  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );

  // Status: accept both forms.
  server.on(
    "/api/status",
    HTTP_GET,
    handleStatus
  );

  server.on(
    "/api/status/",
    HTTP_GET,
    handleStatus
  );

  // Config: accept both forms.
  server.on(
    "/api/config",
    HTTP_GET,
    handleConfig
  );

  server.on(
    "/api/config/",
    HTTP_GET,
    handleConfig
  );

  // Mock: accept both forms.
  server.on(
    "/api/mock",
    HTTP_GET,
    handleMock
  );

  server.on(
    "/api/mock/",
    HTTP_GET,
    handleMock
  );

  server.on(
    "/api/mock/off",
    HTTP_GET,
    handleMockOff
  );

  server.on(
    "/api/mock/off/",
    HTTP_GET,
    handleMockOff
  );

  server.onNotFound(
    []() {
      String uri = server.uri();

      Serial.print("Request fallback - URI: [");
      Serial.print(uri);
      Serial.println("]");

      // Normalize a trailing slash so both /api/status and
      // /api/status/ are treated identically.
      while (
        uri.length() > 1 &&
        uri.endsWith("/")
      ) {
        uri.remove(uri.length() - 1);
      }

      // Fallback router. This deliberately handles the API
      // endpoints even if WebServer's registered-route matching
      // does not match the incoming request on this ESP32 build.
      if (uri == "/api/status") {
        handleStatus();
        return;
      }

      if (uri == "/api/config") {
        handleConfig();
        return;
      }

      if (uri == "/api/mock") {
        handleMock();
        return;
      }

      if (uri == "/api/mock/off") {
        handleMockOff();
        return;
      }

      Serial.print("404 - normalized URI: [");
      Serial.print(uri);
      Serial.println("]");

      server.send(
        404,
        "application/json",
        "{\"error\":\"Not found\"}"
      );
    }
  );

  server.begin();

  Serial.println(
    "HTTP server started."
  );

  Serial.println(
    "API status: /api/status"
  );
}

// ============================================================
// WIFI
// ============================================================

void connectWiFi() {
  WiFi.mode(
    WIFI_STA
  );

  WiFi.setAutoReconnect(
    true
  );

  Serial.print(
    "Connecting to Wi-Fi"
  );

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  while (
    WiFi.status() !=
    WL_CONNECTED
  ) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.println(
    "Wi-Fi connected."
  );

  Serial.print(
    "IP address: "
  );

  Serial.println(
    WiFi.localIP()
  );
}

// ============================================================
// TIME
// ============================================================

void setupTime() {
  // United Kingdom:
  // GMT in winter, BST in summer.

  setenv(
    "TZ",
    "GMT0BST,M3.5.0/1,M10.5.0/2",
    1
  );

  tzset();

  configTime(
    0,
    0,
    "pool.ntp.org",
    "time.cloudflare.com",
    "time.google.com"
  );

  Serial.print(
    "Synchronising time"
  );

  constexpr unsigned long TIME_SYNC_TIMEOUT_MS =
    10000;

  unsigned long syncStartedAt =
    millis();

  while (
    time(nullptr) < 100000 &&
    millis() - syncStartedAt < TIME_SYNC_TIMEOUT_MS
  ) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  time_t now =
    time(nullptr);

  if (now < 100000) {
    Serial.println(
      "WARNING: Time synchronisation failed. Continuing startup."
    );

    return;
  }

  Serial.println(
    "Time synchronised."
  );

  struct tm timeInfo;

  if (
    localtime_r(
      &now,
      &timeInfo
    )
  ) {
    Serial.printf(
      "Local time: %02d:%02d:%02d\n",
      timeInfo.tm_hour,
      timeInfo.tm_min,
      timeInfo.tm_sec
    );
  }
}

// ============================================================
// FASTLED SETUP
// ============================================================

void setupFastLED() {
  Serial.println(
    "Initialising FastLED..."
  );

  FastLED.addLeds<
    WS2812B,
    LED_RING_PIN,
    GRB
  >(
    leds,
    LED_RING_COUNT
  );

  FastLED.setBrightness(0);

  FastLED.clear();
  FastLED.show();

  Serial.println(
    "FastLED ready."
  );
}

// ============================================================
// FUTURE HARDWARE SETUP
// ============================================================

void setupFutureHardware() {
  pinMode(
    PUMP_PIN,
    OUTPUT
  );

  digitalWrite(
    PUMP_PIN,
    LOW
  );

  pinMode(
    MISTER_PIN,
    OUTPUT
  );

  digitalWrite(
    MISTER_PIN,
    LOW
  );
}

// ============================================================
// LIGHTNING SETUP
// ============================================================

void setupLightning() {
  if (ENABLE_LIGHTNING_LED) {
    ledcAttach(
      LIGHTNING_LED_PIN,
      5000,
      8
    );

    ledcWrite(
      LIGHTNING_LED_PIN,
      0
    );
  }

  if (ENABLE_LIGHTNING_LED_2) {
    ledcAttach(
      LIGHTNING_LED_2_PIN,
      5000,
      8
    );

    ledcWrite(
      LIGHTNING_LED_2_PIN,
      0
    );
  }
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println(
    "=========================="
  );
  Serial.println(
    "WEATHER JAR BOOT"
  );
  Serial.println(
    "=========================="
  );

  randomSeed(
    analogRead(34)
  );

  setupFastLED();

  setupFutureHardware();

  setupLightning();

  connectWiFi();

  setupTime();

  updateWeather();

  lastWeatherUpdate =
    millis();

  setupServer();

  Serial.println(
    "Applying initial weather light..."
  );

  ledState =
    calculateLedState();

  applyLedState(
    ledState
  );

  lastLedUpdate =
    millis();

  Serial.println(
    "Initial weather light applied."
  );

  Serial.println(
    "=========================="
  );
  Serial.println(
    "SETUP COMPLETE"
  );
  Serial.println(
    "=========================="
  );
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  server.handleClient();

  updateMockExpiration();

  // ----------------------------------------------------------
  // WEATHER
  // ----------------------------------------------------------

  if (
    millis() -
    lastWeatherUpdate >=
    WEATHER_UPDATE_INTERVAL
  ) {
    if (updateWeather()) {
      LedState newState =
        calculateLedState();

      if (
        ledOutputChanged(
          ledState,
          newState
        )
      ) {
        applyLedState(
          newState
        );
      }

      ledState =
        newState;
    }

    lastWeatherUpdate =
      millis();
  }

  // ----------------------------------------------------------
  // WEATHER RING
  // ----------------------------------------------------------

  if (
    millis() -
    lastLedUpdate >=
    LED_RING_UPDATE_INTERVAL
  ) {
    LedState newState =
      calculateLedState();

    if (
      ledOutputChanged(
        ledState,
        newState
      )
    ) {
      applyLedState(
        newState
      );
    }

    ledState =
      newState;

    lastLedUpdate =
      millis();
  }

  // ----------------------------------------------------------
  // LIGHTNING
  // ----------------------------------------------------------

  updateLightning();

  delay(5);
}