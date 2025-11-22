#include "app_state.h"

ESP8266WebServer server(80);
DNSServer dnsServer;

String wifiSsid = DEFAULT_WIFI_SSID;
String wifiPassword = DEFAULT_WIFI_PASSWORD;
String portalNetworkOptions;

LightSchedule scheduleConfig = {8, 0, 20, 0, true};
bool storageReady = false;
bool captivePortalActive = false;
bool scheduleWindowActive = false;
uint8_t userBrightnessPercent = 70;
uint8_t appliedBrightnessPercent = 0;
int timezoneOffsetMinutes = 0;
unsigned long lastScheduleCheck = 0;
TimezoneRules timezoneRules = {false, 0, 0, String("UTC"), String("")};
