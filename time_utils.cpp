#include "time_utils.h"

#include <time.h>
#include <stdlib.h>

#include "app_state.h"

namespace {

String formatOffsetString(int offset) {
  // Offset is stored as minutes from UTC, where negative means west of UTC
  const char sign = offset >= 0 ? '+' : '-';
  int absMinutes = abs(offset);
  int hours = absMinutes / 60;
  int minutes = absMinutes % 60;
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "UTC%c%02d:%02d", sign, hours, minutes);
  return String(buffer);
}

}

uint16_t minutesSinceMidnight() {
  const time_t now = time(nullptr);
  if (now >= MIN_VALID_EPOCH) {
    struct tm timeInfo;
    localtime_r(&now, &timeInfo);
    return static_cast<uint16_t>(timeInfo.tm_hour * 60 + timeInfo.tm_min);
  }
  long minutes = static_cast<long>(millis() / 60000UL);
  minutes += timezoneOffsetMinutes;
  minutes %= 1440;
  if (minutes < 0) {
    minutes += 1440;
  }
  return static_cast<uint16_t>(minutes);
}

bool isTimeValid() {
  return time(nullptr) >= MIN_VALID_EPOCH;
}

String formatTimezoneLabel() {
  String label = timezoneRules.ianaName.length() ? timezoneRules.ianaName : String("UTC");
  label += ' ';
  label += '(';
  label += formatOffsetString(timezoneOffsetMinutes);
  label += ')';
  return label;
}

String buildLocalTimeString() {
  time_t now = time(nullptr);
  if (now < MIN_VALID_EPOCH) {
    return String("--:--");
  }
  struct tm timeInfo;
  localtime_r(&now, &timeInfo);
  char buffer[24];
  strftime(buffer, sizeof(buffer), "%I:%M:%S %p", &timeInfo);
  return String(buffer);
}

int clampTimezoneOffset(int minutes) {
  if (minutes < -720) {
    return -720;
  }
  if (minutes > 840) {
    return 840;
  }
  return minutes;
}

void applyTimezoneConfig() {
  Serial.print(F("[Time] Current UTC time: "));
  time_t utcTime = time(nullptr);
  if (utcTime >= MIN_VALID_EPOCH) {
    struct tm utcInfo;
    gmtime_r(&utcTime, &utcInfo);
    char utcBuffer[24];
    strftime(utcBuffer, sizeof(utcBuffer), "%Y-%m-%d %H:%M:%S UTC", &utcInfo);
    Serial.println(utcBuffer);
  } else {
    Serial.println("Invalid");
  }
  
  if (timezoneRules.posixSpec.length()) {
    String tzSpec = timezoneRules.posixSpec;
    if (!tzSpec.startsWith(":")) {
      tzSpec = ":" + tzSpec;
    }
    setenv("TZ", tzSpec.c_str(), 1);
    tzset();
    // Don't call configTime here - NTP is already configured and gives us UTC
    // The POSIX timezone string handles the local conversion
    Serial.print(F("[Time] Applied POSIX timezone: "));
    Serial.println(tzSpec);
  } else {
    // For simple offset-based timezones, create a basic POSIX string
    // Format: STD offset (e.g., "MST7" for UTC-7)
    String tzSpec = "LOC";
    int offsetHours = timezoneOffsetMinutes / 60;
    int offsetMins = abs(timezoneOffsetMinutes % 60);
    
    // POSIX timezone offsets are inverted from UTC offsets
    offsetHours = -offsetHours;
    
    tzSpec += String(offsetHours);
    if (offsetMins != 0) {
      tzSpec += ":" + String(offsetMins);
    }
    
    setenv("TZ", tzSpec.c_str(), 1);
    tzset();
    Serial.print(F("[Time] Applied simple timezone: "));
    Serial.println(tzSpec);
  }
  
  // Show current time after timezone change
  delay(100); // Brief delay to let timezone take effect
  Serial.print(F("[Time] Local time is now: "));
  Serial.println(buildLocalTimeString());
}
