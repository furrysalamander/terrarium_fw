#include "web_routes.h"

#include <ESP8266WebServer.h>

#include "app_state.h"
#include "lighting.h"
#include "portal.h"
#include "storage.h"
#include "time_utils.h"
#include "utils.h"
#include "web_assets.h"

namespace {

String buildPortalInstructions() {
  String text = "When active, join &ldquo;";
  text += PORTAL_AP_SSID;
  text += "&rdquo; (password: ";
  text += PORTAL_AP_PASSWORD;
  text += ").";
  return text;
}

String buildIndexPage() {
  String page = loadIndexTemplate();
  page.replace("{{BRIGHTNESS}}", String(userBrightnessPercent));
  page.replace("{{SCHEDULE_CHECKED}}", scheduleConfig.enabled ? "checked" : "");
  page.replace("{{ON_TIME}}", formatTimeField(scheduleConfig.onHour, scheduleConfig.onMinute));
  page.replace("{{OFF_TIME}}", formatTimeField(scheduleConfig.offHour, scheduleConfig.offMinute));
  page.replace("{{LOCAL_IP}}", WiFi.localIP().toString());
  page.replace("{{TIMEZONE_OFFSET}}", String(timezoneOffsetMinutes));
  page.replace("{{TIMEZONE_LABEL}}", formatTimezoneLabel());
  page.replace("{{PORTAL_INSTRUCTIONS}}", buildPortalInstructions());
  return page;
}

String buildPortalPage() {
  String page = loadPortalTemplate();
  page.replace("{{PORTAL_SSID}}", PORTAL_AP_SSID);
  page.replace("{{PORTAL_PASSWORD}}", PORTAL_AP_PASSWORD);
  if (portalNetworkOptions.length() == 0) {
    page.replace("{{NETWORK_OPTIONS}}", "<option disabled>Scanning...</option>");
  } else {
    page.replace("{{NETWORK_OPTIONS}}", portalNetworkOptions);
  }
  return page;
}

void handleRoot() {
  if (portalRequestShouldSeeSetup()) {
    server.send(200, "text/html", buildPortalPage());
    return;
  }
  server.send(200, "text/html", buildIndexPage());
}

void handleScheduleUpdate() {
  bool parsed = true;
  if (server.hasArg("onTime")) {
    parsed &= parseTimeArg(server.arg("onTime"), scheduleConfig.onHour, scheduleConfig.onMinute);
  } else {
    parsed = false;
  }
  if (server.hasArg("offTime")) {
    parsed &= parseTimeArg(server.arg("offTime"), scheduleConfig.offHour, scheduleConfig.offMinute);
  } else {
    parsed = false;
  }

  scheduleConfig.enabled = server.hasArg("enabled");

  if (!parsed) {
    server.send(400, "text/plain", "Invalid time format");
    return;
  }

  enforceOutputFromState(true);
  savePersistentConfig();
  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "");
}

void handleBrightnessUpdate() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Missing value");
    return;
  }

  int requested = server.arg("value").toInt();
  userBrightnessPercent = clampPercent(requested);
  enforceOutputFromState(true);
  savePersistentConfig();

  server.send(200, "application/json", "{\"ok\":true}");
}

void handleStatus() {
  String payload = "{";
  payload += "\"brightness\":" + String(userBrightnessPercent) + ',';
  payload += "\"appliedBrightness\":" + String(appliedBrightnessPercent) + ',';
  payload += "\"lightOn\":";
  payload += (appliedBrightnessPercent > 0 ? "true," : "false,");
  payload += "\"clockSynced\":";
  payload += (isTimeValid() ? "true," : "false,");
  payload += "\"localTime\":\"" + jsonEscape(buildLocalTimeString()) + "\",";
  payload += "\"timezoneOffset\":" + String(timezoneOffsetMinutes) + ',';
  payload += "\"timezoneLabel\":\"" + jsonEscape(formatTimezoneLabel()) + "\",";
  payload += "\"timezoneName\":\"" + jsonEscape(timezoneRules.ianaName) + "\",";
  payload += "\"schedule\":{";
  payload += "\"enabled\":";
  payload += (scheduleConfig.enabled ? "true," : "false,");
  payload += "\"active\":";
  payload += (scheduleWindowActive ? "true," : "false,");
  payload += "\"on\":\"" + formatTimeField(scheduleConfig.onHour, scheduleConfig.onMinute) + "\",";
  payload += "\"off\":\"" + formatTimeField(scheduleConfig.offHour, scheduleConfig.offMinute) + "\"},";
  payload += "\"wifi\":{";
  payload += "\"connected\":";
  payload += (WiFi.status() == WL_CONNECTED ? "true," : "false,");
  payload += "\"ssid\":\"" + jsonEscape(wifiSsid) + "\"},";
  payload += "\"portal\":{";
  payload += "\"active\":";
  payload += (captivePortalActive ? "true," : "false,");
  payload += "\"ssid\":\"" + jsonEscape(String(PORTAL_AP_SSID)) + "\",";
  payload += "\"password\":\"" + jsonEscape(String(PORTAL_AP_PASSWORD)) + "\"},";
  payload += "\"localIp\":\"" + WiFi.localIP().toString() + "\"";
  payload += '}';

  server.send(200, "application/json", payload);
}

void handlePortalStart() {
  bool started = startCaptivePortal();
  String message = started ? String("Portal running. Connect to \"") + PORTAL_AP_SSID + "\" (pass: " + PORTAL_AP_PASSWORD + ")." : String("Unable to start portal. Please retry.");
  String payload = "{";
  payload += "\"active\":";
  payload += (started ? "true," : "false,");
  payload += "\"ssid\":\"" + jsonEscape(String(PORTAL_AP_SSID)) + "\",";
  payload += "\"password\":\"" + jsonEscape(String(PORTAL_AP_PASSWORD)) + "\",";
  payload += "\"message\":\"" + jsonEscape(message) + "\"";
  payload += '}';
  server.send(started ? 200 : 500, "application/json", payload);
}

void handlePortalPage() {
  server.send(200, "text/html", buildPortalPage());
}

void handlePortalSave() {
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "text/plain", "Missing ssid/password");
    return;
  }
  String newSsid = server.arg("ssid");
  String newPass = server.arg("password");
  newSsid.trim();
  newPass.trim();
  if (newSsid.length() == 0 || newSsid.length() > 32) {
    server.send(400, "text/plain", "SSID must be 1-32 characters");
    return;
  }
  if (newPass.length() < 8 || newPass.length() > 63) {
    server.send(400, "text/plain", "Password must be 8-63 characters");
    return;
  }
  wifiSsid = newSsid;
  wifiPassword = newPass;
  savePersistentConfig();
  stopCaptivePortal();
  server.send(200, "text/html", "<html><body><h2>Saved credentials.</h2><p>Rebooting...</p></body></html>");
  delay(1500);
  ESP.restart();
}

void handleTimezoneUpdate() {
  if (!server.hasArg("offset")) {
    server.send(400, "text/plain", "Missing offset");
    return;
  }
  timezoneOffsetMinutes = clampTimezoneOffset(server.arg("offset").toInt());
  Serial.print(F("[Timezone] Received offset: "));
  Serial.println(timezoneOffsetMinutes);
  if (server.hasArg("stdOffset")) {
    timezoneRules.standardOffsetMinutes = clampTimezoneOffset(server.arg("stdOffset").toInt());
  } else {
    timezoneRules.standardOffsetMinutes = timezoneOffsetMinutes;
  }
  if (server.hasArg("dstOffset")) {
    timezoneRules.daylightOffsetMinutes = clampTimezoneOffset(server.arg("dstOffset").toInt());
  } else {
    timezoneRules.daylightOffsetMinutes = timezoneRules.standardOffsetMinutes;
  }
  timezoneRules.hasDst = server.hasArg("hasDst") && server.arg("hasDst") != "0";
  if (server.hasArg("tzName")) {
    timezoneRules.ianaName = server.arg("tzName");
  }
  if (timezoneRules.hasDst && server.hasArg("posix") && server.arg("posix").length()) {
    timezoneRules.posixSpec = server.arg("posix");
    Serial.print(F("[Timezone] Using POSIX spec: "));
    Serial.println(timezoneRules.posixSpec);
  } else if (!timezoneRules.hasDst) {
    timezoneRules.posixSpec = "";
    Serial.println(F("[Timezone] No DST, using simple offset"));
  }
  Serial.print(F("[Timezone] Timezone name: "));
  Serial.println(timezoneRules.ianaName);
  applyTimezoneConfig();
  savePersistentConfig();
  String payload = "{";
  payload += "\"timezoneOffset\":" + String(timezoneOffsetMinutes) + ',';
  payload += "\"timezoneLabel\":\"" + jsonEscape(formatTimezoneLabel()) + "\",";
  payload += "\"timezoneName\":\"" + jsonEscape(timezoneRules.ianaName) + "\"";
  payload += '}';
  server.send(200, "application/json", payload);
}

}  // namespace

void configureRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/schedule", HTTP_POST, handleScheduleUpdate);
  server.on("/brightness", HTTP_POST, handleBrightnessUpdate);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/wifi/portal", HTTP_POST, handlePortalStart);
  server.on("/portal", HTTP_GET, handlePortalPage);
  server.on("/portal/save", HTTP_POST, handlePortalSave);
  server.on("/timezone", HTTP_POST, handleTimezoneUpdate);

  server.on("/assets/main.css", HTTP_GET, []() { sendMainCss(); });
  server.on("/assets/app.js", HTTP_GET, []() { sendAppJs(); });
  server.on("/assets/portal.css", HTTP_GET, []() { sendPortalCss(); });
  server.on("/assets/portal.js", HTTP_GET, []() { sendPortalJs(); });

  server.onNotFound([]() {
    if (portalRequestShouldSeeSetup()) {
      server.send(200, "text/html", buildPortalPage());
      return;
    }
    server.send(404, "text/plain", "Not found");
  });
  server.begin();
}
