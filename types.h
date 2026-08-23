#pragma once

#include <Arduino.h>

// ============================================================
// ENUMS
// ============================================================

enum class WeatherType {
  CLEAR,
  MAINLY_CLEAR,
  PARTLY_CLOUDY,
  OVERCAST,
  FOG,
  DRIZZLE,
  RAIN,
  SNOW,
  THUNDERSTORM,
  UNKNOWN
};

enum class DayPeriod {
  NIGHT,
  SUNRISE,
  DAY,
  SUNSET
};

enum class SunVisibility {
  FULL,
  PARTIAL,
  NONE
};

// ============================================================
// COLOR
// ============================================================

struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// ============================================================
// WEATHER STATE
// ============================================================

struct WeatherState {
  float temperature = 0.0f;
  int weatherCode = -1;

  float precipitation = 0.0f;
  float rain = 0.0f;

  WeatherType weather = WeatherType::UNKNOWN;

  time_t sunrise = 0;
  time_t sunset = 0;

  bool valid = false;
};

// ============================================================
// MOCK STATE
// ============================================================

struct MockState {
  bool enabled = false;

  bool overrideWeather = false;
  WeatherType weather = WeatherType::UNKNOWN;

  bool overridePeriod = false;
  DayPeriod period = DayPeriod::DAY;

  unsigned long startedAt = 0;
  unsigned long expiresAt = 0;

  float speed = 1.0f;
};

// ============================================================
// LED STATE
// ============================================================

struct LedState {
  RgbColor color = {0, 0, 0};

  uint8_t brightness = 0;

  String effect = "off";

  float progress = 0.0f;
};

// ============================================================
// LIGHTNING STATE
// ============================================================

struct LightningState {
  bool active = false;
  bool inFlash = false;

  uint8_t brightness = 0;
  uint8_t brightness2 = 0;

  int flashesRemaining = 0;

  unsigned long nextEventAt = 0;
  unsigned long stateUntil = 0;

  // Second LED follows the first LED without blocking the main loop.
  bool led2Pending = false;
  bool led2InFlash = false;
  uint8_t led2PendingBrightness = 0;
  unsigned long led2StartAt = 0;
  unsigned long led2EndAt = 0;
};