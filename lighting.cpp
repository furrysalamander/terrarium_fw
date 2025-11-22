#include "lighting.h"

#include <Arduino.h>
#include <math.h>

#include "time_utils.h"

namespace {
constexpr float OUTPUT_GAMMA = 2.2f;
constexpr unsigned long BREATHING_UPDATE_INTERVAL_MS = 5;
constexpr unsigned long BREATHING_CYCLE_DURATION_MS = 6000;
constexpr uint8_t BREATHING_MIN_BRIGHTNESS = 5;
constexpr uint8_t BREATHING_MAX_BRIGHTNESS = 60;
}

void enforceOutputFromState(bool forceUpdate) {
  uint16_t nowMinutes = minutesSinceMidnight();
  scheduleWindowActive = scheduleConfig.enabled && isWithinSchedule(nowMinutes);
  bool lightAllowed = !scheduleConfig.enabled || scheduleWindowActive;
  uint8_t target = lightAllowed ? userBrightnessPercent : 0;

  if (forceUpdate || target != appliedBrightnessPercent) {
    appliedBrightnessPercent = target;
    applyLightOutput(target);
  }
}

void applyLightOutput(float percent) {
  if (percent < 0.0f) {
    percent = 0.0f;
  } else if (percent > 100.0f) {
    percent = 100.0f;
  }

  const float normalized = percent / 100.0f;
  const float gammaCorrected = normalized <= 0.0f ? 0.0f : powf(normalized, OUTPUT_GAMMA);
  const float inverted = 1.0f - gammaCorrected;
  const uint16_t pwmValue = static_cast<uint16_t>(lroundf(inverted * MAX_ANALOG_WRITE));
  analogWrite(LIGHT_PIN, pwmValue);
  Serial.println(F("[Light] Set brightness to ") + String(percent, 2) + F("% (PWM=") + String(pwmValue) + F(")"));
  digitalWrite(STATUS_LED_PIN, percent > 0.5f ? HIGH : LOW);
}

void updateBreathingEffect() {
  static unsigned long lastBreathingUpdate = 0;
  const unsigned long now = millis();

  if (now - lastBreathingUpdate < BREATHING_UPDATE_INTERVAL_MS) {
    return;
  }
  lastBreathingUpdate = now;

  const float cycle = (now % BREATHING_CYCLE_DURATION_MS) / static_cast<float>(BREATHING_CYCLE_DURATION_MS);
  const float phase = cycle * 2.0f * M_PI;
  const float easedWave = 0.5f - 0.5f * cosf(phase);
  const float brightnessSpan = static_cast<float>(BREATHING_MAX_BRIGHTNESS - BREATHING_MIN_BRIGHTNESS);
  const float targetPercent = BREATHING_MIN_BRIGHTNESS + (easedWave * brightnessSpan);

  applyLightOutput(targetPercent);

  static unsigned long lastDebug = 0;
  if (now - lastDebug >= 1000) {
    lastDebug = now;
    Serial.print(F("[Breathing] cycle="));
    Serial.print(cycle, 3);
    Serial.print(F(" eased="));
    Serial.print(easedWave, 3);
    Serial.print(F(" percent="));
    Serial.println(targetPercent, 2);
  }
}

bool isWithinSchedule(uint16_t currentMinutes) {
  const uint16_t onMinutes = scheduleConfig.onHour * 60 + scheduleConfig.onMinute;
  const uint16_t offMinutes = scheduleConfig.offHour * 60 + scheduleConfig.offMinute;

  if (onMinutes == offMinutes) {
    return false;
  }

  if (onMinutes < offMinutes) {
    return currentMinutes >= onMinutes && currentMinutes < offMinutes;
  }
  return currentMinutes >= onMinutes || currentMinutes < offMinutes;
}

bool parseTimeArg(const String &value, uint8_t &hourOut, uint8_t &minuteOut) {
  if (value.length() != 5 || value.charAt(2) != ':') {
    return false;
  }
  int hour = value.substring(0, 2).toInt();
  int minute = value.substring(3).toInt();
  if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
    return false;
  }
  hourOut = static_cast<uint8_t>(hour);
  minuteOut = static_cast<uint8_t>(minute);
  return true;
}

String formatTimeField(uint8_t hour, uint8_t minute) {
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02u:%02u", hour, minute);
  return String(buffer);
}

uint8_t clampPercent(int value) {
  if (value < 0) {
    return 0;
  }
  if (value > 100) {
    return 100;
  }
  return static_cast<uint8_t>(value);
}
