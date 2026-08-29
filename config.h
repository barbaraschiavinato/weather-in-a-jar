#pragma once

#include <Arduino.h>

// ============================================================
// WEATHER JAR - HARDWARE CONFIGURATION
// ============================================================


// ============================================================
// LED RING - WEATHER / SUNRISE / SUNSET
// ============================================================

constexpr int LED_RING_PIN = 18;
constexpr int LED_RING_COUNT = 35;
constexpr bool ENABLE_RING_LIGHTNING = true;


// 100 ms = maximum 10 FPS.
// More than enough for very slow weather / solar fades.
constexpr unsigned long LED_RING_UPDATE_INTERVAL = 100;


// ============================================================
// LIGHTNING LEDs - WHITE LEDs
// ============================================================

constexpr int LIGHTNING_LED_PIN = 23;
constexpr bool ENABLE_LIGHTNING_LED = false;

// Optional second lightning LED.
// It reproduces each flash after a configurable delay.
constexpr int LIGHTNING_LED_2_PIN = 22;
constexpr bool ENABLE_LIGHTNING_LED_2 = false;
constexpr unsigned long LIGHTNING_LED_2_DELAY_MS = 35;

constexpr uint8_t LIGHTNING_MIN_BRIGHTNESS = 180;
constexpr uint8_t LIGHTNING_MAX_BRIGHTNESS = 255;

constexpr unsigned long LIGHTNING_FLASH_MIN_MS = 20;
constexpr unsigned long LIGHTNING_FLASH_MAX_MS = 90;

constexpr unsigned long LIGHTNING_GAP_MIN_MS = 40;
constexpr unsigned long LIGHTNING_GAP_MAX_MS = 150;

constexpr unsigned long LIGHTNING_EVENT_MIN_MS = 4000;
constexpr unsigned long LIGHTNING_EVENT_MAX_MS = 20000;

constexpr int LIGHTNING_MAX_FLASHES_PER_EVENT = 3;


// ============================================================
// FUTURE WATER HARDWARE
// ============================================================

// These GPIOs will control MOSFETs / drivers.
// Do NOT connect pump or mister power directly to the ESP32.

constexpr int PUMP_PIN = 19;
constexpr int MISTER_PIN = 21;

constexpr bool ENABLE_PUMP = false;
constexpr bool ENABLE_MISTER = false;


// ============================================================
// AUTOMATIC MOCK LOOP
// ============================================================

// Each visual mock runs for 10 seconds.
constexpr unsigned long MOCK_LOOP_STEP_MS =
  10UL * 1000UL;

// Ring completely off between two mock effects.
constexpr unsigned long MOCK_LOOP_BLACKOUT_MS =
  1000UL;

// clear, mainly_clear, partly_cloudy, overcast, fog, drizzle,
// rain, snow, thunderstorm, sunrise, sunset
constexpr int MOCK_LOOP_COUNT = 11;


// ============================================================
// WEATHER
// ============================================================

// Refresh Open-Meteo every 15 minutes.
constexpr unsigned long WEATHER_UPDATE_INTERVAL =
  15UL * 60UL * 1000UL;


// ============================================================
// SUNRISE / SUNSET
// ============================================================

// Effect:
// 30 minutes before sunrise/sunset
// +
// 30 minutes after
//
// Total effect duration = 60 minutes.

constexpr long SOLAR_EFFECT_HALF_WINDOW_SECONDS =
  30L * 60L;


// ============================================================
// WEATHER LIGHT
// ============================================================

constexpr bool ENABLE_WEATHER_LIGHT = true;
constexpr bool ENABLE_SOLAR_EFFECTS = true;


