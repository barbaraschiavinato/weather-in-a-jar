#pragma once

#include <Arduino.h>
#include "types.h"

// ============================================================
// WEATHER JAR - VISUAL CONFIGURATION
// ============================================================

constexpr RgbColor COLOR_OFF = { 0, 0, 0 };
constexpr RgbColor COLOR_WARM_WHITE = { 255, 160, 80 };
constexpr RgbColor COLOR_COOL_WHITE = { 200, 220, 255 };
constexpr RgbColor COLOR_MAINLY_CLEAR = { 245, 205, 150 };
constexpr RgbColor COLOR_PARTLY_CLOUDY = { 220, 220, 205 };
constexpr RgbColor COLOR_OVERCAST = { 95, 110, 125 };
constexpr RgbColor COLOR_FOG = { 150, 165, 170 };
constexpr RgbColor COLOR_DRIZZLE = { 70, 125, 180 };
constexpr RgbColor COLOR_RAIN = { 35, 90, 180 };
constexpr RgbColor COLOR_SNOW = { 210, 225, 255 };
constexpr RgbColor COLOR_STORM = { 25, 35, 80 };

// SUNRISE / SUNSET COLORS
constexpr RgbColor COLOR_DEEP_RED = { 150, 10, 0 };
constexpr RgbColor COLOR_RED = { 230, 35, 5 };
constexpr RgbColor COLOR_ORANGE = { 255, 90, 10 };
constexpr RgbColor COLOR_PINK = { 255, 105, 180 };
constexpr RgbColor COLOR_PURPLE = { 138, 43, 226 };
constexpr RgbColor COLOR_AMBER = { 255, 155, 35 };

constexpr RgbColor COLOR_PARTIAL_DEEP = { 120, 35, 15 };
constexpr RgbColor COLOR_PARTIAL_ORANGE = { 210, 95, 40 };
constexpr RgbColor COLOR_PARTIAL_PINK = { 220, 105, 145 };
constexpr RgbColor COLOR_PARTIAL_PURPLE = { 105, 65, 145 };
constexpr RgbColor COLOR_PARTIAL_WARM = { 230, 175, 120 };

// WEATHER BRIGHTNESS
constexpr uint8_t BRIGHTNESS_CLEAR = 180;
constexpr uint8_t BRIGHTNESS_MAINLY_CLEAR = 170;
constexpr uint8_t BRIGHTNESS_PARTLY_CLOUDY = 150;
constexpr uint8_t BRIGHTNESS_OVERCAST = 110;
constexpr uint8_t BRIGHTNESS_FOG = 90;
constexpr uint8_t BRIGHTNESS_DRIZZLE = 125;
constexpr uint8_t BRIGHTNESS_RAIN = 120;
constexpr uint8_t BRIGHTNESS_SNOW = 150;
constexpr uint8_t BRIGHTNESS_THUNDERSTORM = 45;
constexpr uint8_t BRIGHTNESS_UNKNOWN = 100;

inline RgbColor getWeatherColor(WeatherType weather) {
  switch (weather) {
    case WeatherType::CLEAR: return COLOR_WARM_WHITE;
    case WeatherType::MAINLY_CLEAR: return COLOR_MAINLY_CLEAR;
    case WeatherType::PARTLY_CLOUDY: return COLOR_PARTLY_CLOUDY;
    case WeatherType::OVERCAST: return COLOR_OVERCAST;
    case WeatherType::FOG: return COLOR_FOG;
    case WeatherType::DRIZZLE: return COLOR_DRIZZLE;
    case WeatherType::RAIN: return COLOR_RAIN;
    case WeatherType::SNOW: return COLOR_SNOW;
    case WeatherType::THUNDERSTORM: return COLOR_STORM;
    default: return COLOR_COOL_WHITE;
  }
}

inline uint8_t getWeatherBrightness(WeatherType weather) {
  switch (weather) {
    case WeatherType::CLEAR: return BRIGHTNESS_CLEAR;
    case WeatherType::MAINLY_CLEAR: return BRIGHTNESS_MAINLY_CLEAR;
    case WeatherType::PARTLY_CLOUDY: return BRIGHTNESS_PARTLY_CLOUDY;
    case WeatherType::OVERCAST: return BRIGHTNESS_OVERCAST;
    case WeatherType::FOG: return BRIGHTNESS_FOG;
    case WeatherType::DRIZZLE: return BRIGHTNESS_DRIZZLE;
    case WeatherType::RAIN: return BRIGHTNESS_RAIN;
    case WeatherType::SNOW: return BRIGHTNESS_SNOW;
    case WeatherType::THUNDERSTORM: return BRIGHTNESS_THUNDERSTORM;
    default: return BRIGHTNESS_UNKNOWN;
  }
}

inline SunVisibility getSunVisibility(WeatherType weather) {
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

constexpr RgbColor SUNRISE_FULL_COLORS[] = {
  COLOR_OFF, COLOR_DEEP_RED, COLOR_RED, COLOR_PINK,
  COLOR_AMBER, COLOR_WARM_WHITE
};
constexpr int SUNRISE_FULL_COLOR_COUNT =
  sizeof(SUNRISE_FULL_COLORS) / sizeof(SUNRISE_FULL_COLORS[0]);

constexpr RgbColor SUNRISE_PARTIAL_COLORS[] = {
  COLOR_OFF, COLOR_PARTIAL_DEEP, COLOR_PARTIAL_PINK, COLOR_PARTIAL_WARM
};
constexpr int SUNRISE_PARTIAL_COLOR_COUNT =
  sizeof(SUNRISE_PARTIAL_COLORS) / sizeof(SUNRISE_PARTIAL_COLORS[0]);

constexpr RgbColor SUNSET_FULL_COLORS[] = {
  COLOR_WARM_WHITE, COLOR_AMBER, COLOR_ORANGE, COLOR_RED,
  COLOR_PURPLE, COLOR_OFF
};
constexpr int SUNSET_FULL_COLOR_COUNT =
  sizeof(SUNSET_FULL_COLORS) / sizeof(SUNSET_FULL_COLORS[0]);

constexpr RgbColor SUNSET_PARTIAL_COLORS[] = {
  COLOR_PARTIAL_WARM, COLOR_PARTIAL_ORANGE, COLOR_PARTIAL_PURPLE, COLOR_OFF
};
constexpr int SUNSET_PARTIAL_COLOR_COUNT =
  sizeof(SUNSET_PARTIAL_COLORS) / sizeof(SUNSET_PARTIAL_COLORS[0]);
