#include "portal.h"

#include <ESP8266WiFi.h>

#include "app_state.h"
#include "lighting.h"
#include "utils.h"

bool startCaptivePortal() {
  if (captivePortalActive) {
    return true;
  }
  WiFi.mode(WIFI_AP_STA);
  bool apStarted = WiFi.softAP(PORTAL_AP_SSID, PORTAL_AP_PASSWORD);
  if (!apStarted) {
    Serial.println(F("[Portal] Failed to start SoftAP."));
    return false;
  }
  IPAddress apIp = WiFi.softAPIP();
  dnsServer.stop();
  dnsServer.start(DNS_PORT, "*", apIp);
  refreshPortalNetworkOptions();
  captivePortalActive = true;
  Serial.print(F("[Portal] Captive portal active at http://"));
  Serial.println(apIp);
  return true;
}

void stopCaptivePortal() {
  if (!captivePortalActive) {
    return;
  }
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  captivePortalActive = false;
  
  // Restore normal lighting control when portal stops
  enforceOutputFromState(true);
  
  Serial.println(F("[Portal] Captive portal stopped."));
}

bool portalRequestShouldSeeSetup() {
  if (!captivePortalActive) {
    return false;
  }
  WiFiClient client = server.client();
  if (!client) {
    return true;
  }
  return client.localIP() == WiFi.softAPIP();
}

void refreshPortalNetworkOptions() {
  portalNetworkOptions = "";
  int networkCount = WiFi.scanNetworks(false, true);
  if (networkCount <= 0) {
    return;
  }
  const int maxOptions = 15;
  for (int i = 0; i < networkCount && i < maxOptions; ++i) {
    String ssid = WiFi.SSID(i);
    if (!ssid.length()) {
      continue;
    }
    String option = "<option value=\"";
    option += htmlEscape(ssid);
    option += "\">";
    option += htmlEscape(ssid);
    option += "</option>";
    portalNetworkOptions += option;
  }
}
