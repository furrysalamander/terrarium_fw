#pragma once

#include <Arduino.h>

uint16_t minutesSinceMidnight();
bool isTimeValid();
String formatTimezoneLabel();
String buildLocalTimeString();
int clampTimezoneOffset(int minutes);
void applyTimezoneConfig();
