#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <time.h>

struct LightSchedule {
  uint8_t onHour;
  uint8_t onMinute;
  uint8_t offHour;
  uint8_t offMinute;
  bool enabled;
};

struct TimezoneRules {
  bool hasDst;
  int16_t standardOffsetMinutes;
  int16_t daylightOffsetMinutes;
  String ianaName;
  String posixSpec;
};

inline constexpr uint8_t LIGHT_PIN = 5;
inline constexpr uint8_t STATUS_LED_PIN = 16;
inline constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000;
inline constexpr uint8_t DNS_PORT = 53;
inline constexpr time_t MIN_VALID_EPOCH = 1609459200; // 2021-01-01
inline constexpr unsigned long SCHEDULE_CHECK_INTERVAL_MS = 1000;
inline constexpr char PORTAL_AP_SSID[] = "Terrarium-Setup";
inline constexpr char PORTAL_AP_PASSWORD[] = "terra1234";
inline constexpr char DEFAULT_WIFI_SSID[] = "";         // Populated via portal or config file
inline constexpr char DEFAULT_WIFI_PASSWORD[] = "";
inline constexpr char CONFIG_PATH[] = "/light_config.csv";

extern ESP8266WebServer server;
extern DNSServer dnsServer;

extern String wifiSsid;
extern String wifiPassword;
extern String portalNetworkOptions;

extern LightSchedule scheduleConfig;
extern bool storageReady;
extern bool captivePortalActive;
extern bool scheduleWindowActive;
extern uint8_t userBrightnessPercent;
extern uint8_t appliedBrightnessPercent;
extern int timezoneOffsetMinutes;
extern unsigned long lastScheduleCheck;
extern TimezoneRules timezoneRules;
