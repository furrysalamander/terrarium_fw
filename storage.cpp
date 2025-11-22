#include "storage.h"

#include <LittleFS.h>

#include "app_state.h"
#include "lighting.h"
#include "time_utils.h"

namespace {

bool parseScheduleValue(const String &value) {
  int firstComma = value.indexOf(',');
  int secondComma = value.indexOf(',', firstComma + 1);
  if (firstComma < 0 || secondComma < 0) {
    return false;
  }
  String onValue = value.substring(0, firstComma);
  String offValue = value.substring(firstComma + 1, secondComma);
  String enabledValue = value.substring(secondComma + 1);

  uint8_t onH, onM, offH, offM;
  if (!parseTimeArg(onValue, onH, onM)) {
    return false;
  }
  if (!parseTimeArg(offValue, offH, offM)) {
    return false;
  }

  scheduleConfig.onHour = onH;
  scheduleConfig.onMinute = onM;
  scheduleConfig.offHour = offH;
  scheduleConfig.offMinute = offM;
  scheduleConfig.enabled = enabledValue.toInt() != 0;
  return true;
}

String sanitizeForConfig(const String &value) {
  String cleaned;
  cleaned.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value.charAt(i);
    if (c == '\r' || c == '\n') {
      continue;
    }
    cleaned += c;
  }
  return cleaned;
}

}  // namespace

void initializeStorage() {
  if (storageReady) {
    return;
  }
  storageReady = mountLittleFilesystem();
  if (!storageReady) {
    Serial.println(F("[FS] Storage unavailable; configuration will not persist."));
    return;
  }
  if (loadPersistentConfig()) {
    Serial.println(F("[FS] Configuration restored from flash."));
  } else {
    Serial.println(F("[FS] Using defaults and creating config file."));
    savePersistentConfig();
  }
  applyTimezoneConfig();
}

bool mountLittleFilesystem() {
  if (LittleFS.begin()) {
    return true;
  }
  Serial.println(F("[FS] LittleFS mount failed. Attempting format..."));
  if (LittleFS.format() && LittleFS.begin()) {
    Serial.println(F("[FS] LittleFS format successful."));
    return true;
  }
  Serial.println(F("[FS] Unable to mount LittleFS after format attempt."));
  return false;
}

bool loadPersistentConfig() {
  if (!storageReady || !LittleFS.exists(CONFIG_PATH)) {
    return false;
  }
  File file = LittleFS.open(CONFIG_PATH, "r");
  if (!file) {
    return false;
  }
  bool loaded = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (!line.length() || line.charAt(0) == '#') {
      continue;
    }
    int sep = line.indexOf('=');
    if (sep < 0) {
      continue;
    }
    String key = line.substring(0, sep);
    String value = line.substring(sep + 1);
    key.trim();
    value.trim();
    if (key == "wifiSsid") {
      wifiSsid = value;
      loaded = true;
    } else if (key == "wifiPassword") {
      wifiPassword = value;
      loaded = true;
    } else if (key == "schedule") {
      loaded |= parseScheduleValue(value);
    } else if (key == "brightness") {
      userBrightnessPercent = clampPercent(value.toInt());
      loaded = true;
    } else if (key == "timezoneOffset") {
      timezoneOffsetMinutes = clampTimezoneOffset(value.toInt());
      loaded = true;
    } else if (key == "timezoneName") {
      timezoneRules.ianaName = value.length() ? value : String("UTC");
      loaded = true;
    } else if (key == "timezonePosix") {
      timezoneRules.posixSpec = value;
      loaded = true;
    } else if (key == "timezoneStdOffset") {
      timezoneRules.standardOffsetMinutes = clampTimezoneOffset(value.toInt());
      loaded = true;
    } else if (key == "timezoneDstOffset") {
      timezoneRules.daylightOffsetMinutes = clampTimezoneOffset(value.toInt());
      loaded = true;
    } else if (key == "timezoneHasDst") {
      timezoneRules.hasDst = value.toInt() != 0;
      loaded = true;
    }
  }
  file.close();
  if (!timezoneRules.ianaName.length()) {
    timezoneRules.ianaName = F("UTC");
  }
  if (!timezoneRules.hasDst) {
    timezoneRules.daylightOffsetMinutes = timezoneRules.standardOffsetMinutes;
  }
  return loaded;
}

void savePersistentConfig() {
  if (!storageReady) {
    return;
  }
  File file = LittleFS.open(CONFIG_PATH, "w");
  if (!file) {
    Serial.println(F("[FS] Failed to open config for writing."));
    return;
  }
  file.print(F("wifiSsid="));
  file.println(sanitizeForConfig(wifiSsid));
  file.print(F("wifiPassword="));
  file.println(sanitizeForConfig(wifiPassword));
  file.print(F("schedule="));
  file.print(formatTimeField(scheduleConfig.onHour, scheduleConfig.onMinute));
  file.print(',');
  file.print(formatTimeField(scheduleConfig.offHour, scheduleConfig.offMinute));
  file.print(',');
  file.println(scheduleConfig.enabled ? 1 : 0);
  file.print(F("brightness="));
  file.println(userBrightnessPercent);
  file.print(F("timezoneOffset="));
  file.println(timezoneOffsetMinutes);
  file.print(F("timezoneName="));
  file.println(sanitizeForConfig(timezoneRules.ianaName));
  file.print(F("timezonePosix="));
  file.println(sanitizeForConfig(timezoneRules.posixSpec));
  file.print(F("timezoneStdOffset="));
  file.println(timezoneRules.standardOffsetMinutes);
  file.print(F("timezoneDstOffset="));
  file.println(timezoneRules.daylightOffsetMinutes);
  file.print(F("timezoneHasDst="));
  file.println(timezoneRules.hasDst ? 1 : 0);
  file.close();
}
