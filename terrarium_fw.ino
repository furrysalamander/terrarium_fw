#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>

#include "app_state.h"
#include "lighting.h"
#include "portal.h"
#include "storage.h"
#include "time_utils.h"
#include "web_routes.h"

const char DEVICE_HOSTNAME[] = "terrarium";

void connectToWifi();
void initializeTime();
bool syncTimeFromAtomicClock();

void setup() {
  // Immediately set LED pins to prevent startup flash
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, HIGH);
  digitalWrite(STATUS_LED_PIN, LOW);
  analogWriteRange(MAX_ANALOG_WRITE);
  // analogWrite(LIGHT_PIN, 1023); // 1023 = OFF (inverted PWM)
  
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("[BOOT] Terrarium lighting controller starting..."));

  initializeStorage();
  enforceOutputFromState(true);

  connectToWifi();
  initializeTime();
  applyTimezoneConfig();
  enforceOutputFromState(true);
  configureRoutes();

  Serial.print(F("[HTTP] Ready. Open http://"));
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
  
  if (captivePortalActive) {
    dnsServer.processNextRequest();
    updateBreathingEffect();
  }

  const unsigned long now = millis();
  if (now - lastScheduleCheck >= SCHEDULE_CHECK_INTERVAL_MS) {
    lastScheduleCheck = now;
    if (!captivePortalActive) {
      enforceOutputFromState(false);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    MDNS.update();
  }
}

void connectToWifi() {
  if (wifiSsid.length() == 0) {
    Serial.println(F("[WiFi] No stored credentials. Launching captive portal."));
    startCaptivePortal();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.hostname(DEVICE_HOSTNAME);
  WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
  Serial.print(F("[WiFi] Connecting to "));
  Serial.println(wifiSsid);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[WiFi] Connection failed. Captive portal will be started."));
    startCaptivePortal();
  } else {
    Serial.print(F("[WiFi] Connected. IP: "));
    Serial.println(WiFi.localIP());
    if (MDNS.begin(DEVICE_HOSTNAME)) {
      MDNS.addService("http", "tcp", 80);
      Serial.print(F("[mDNS] Hostname ready at http://"));
      Serial.print(DEVICE_HOSTNAME);
      Serial.println(F(".local"));
    } else {
      Serial.println(F("[mDNS] Failed to start responder."));
    }
  }
}

void initializeTime() {
  if (syncTimeFromAtomicClock()) {
    Serial.println(F("Atomic clock sync complete."));
    return;
  }

  applyTimezoneConfig();
  Serial.println(F("[Time] Syncing via NTP..."));
  for (uint8_t i = 0; i < 30; ++i) {
    if (isTimeValid()) {
      Serial.println(F("[Time] NTP sync complete."));
      return;
    }
    delay(500);
  }
  Serial.println(F("[Time] NTP sync failed, using uptime fallback."));
}

bool syncTimeFromAtomicClock() {
  // Placeholder: integrate the atomic clock receiver module when wiring is ready.
  // Expected workflow: read the module, parse the time code, and call settimeofday().
  return false;
}

