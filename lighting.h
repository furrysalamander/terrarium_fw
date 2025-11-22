#pragma once

#include <Arduino.h>

#include "app_state.h"

const float MAX_ANALOG_WRITE = 1023.0f;

void enforceOutputFromState(bool forceUpdate);
void applyLightOutput(float percent);
bool isWithinSchedule(uint16_t currentMinutes);
bool parseTimeArg(const String &value, uint8_t &hourOut, uint8_t &minuteOut);
String formatTimeField(uint8_t hour, uint8_t minute);
uint8_t clampPercent(int value);
void updateBreathingEffect();
