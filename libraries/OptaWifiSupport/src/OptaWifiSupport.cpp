#include "OptaWifiSupport.h"
#include <WiFi.h>

// Confirmed against the actual installed Opta core (arduino:mbed_opta 4.6.0):
// WiFi.h (WiFiClass) declares beginAP(ssid, passphrase, channel = DEFAULT_AP_CHANNEL) and
// begin(ssid, passphrase, security = ENC_TYPE_UNKNOWN); the 4-arg IPAddress config() overload
// is inherited from MbedSocketClass (SocketHelpers.h) and applies to both AP and station mode,
// not just station mode as originally assumed. wl_status_t (wl_definitions.h) confirms
// WL_CONNECTED/WL_AP_LISTENING/WL_AP_CONNECTED/WL_AP_FAILED as used below.
//
// beginAccessPoint()/beginStation() call into blocking WiFi.beginAP()/begin() calls, which can
// take several seconds. maintainAccessPoint()/maintainStationConnection() call these directly
// when disconnected, so loop() will block for that duration during a reconnect attempt - no
// backoff/rate-limiting is implemented, kept deliberately simple.

namespace {

// Copies of the most recent settings, kept for maintain*() to reattempt with. Stored as
// owned Strings rather than the caller's raw pointers, since the caller's own buffer isn't
// guaranteed to outlive the call.
bool haveApSettings = false;
String apSsid, apPassword;
IPAddress apIp, apSubnet;

bool haveStationSettings = false;
String stationSsid, stationPassword;
IPAddress stationIp, stationSubnet, stationGateway;

}  // namespace

bool beginAccessPoint(const char* ssid, const char* password, IPAddress ip, IPAddress subnet) {
  haveApSettings = true;
  apSsid = ssid;
  apPassword = password;
  apIp = ip;
  apSubnet = subnet;

  WiFi.config(ip, IPAddress(), IPAddress(), subnet);
  int status = WiFi.beginAP(ssid, password);
  if (status != WL_AP_LISTENING && status != WL_AP_CONNECTED) {
    Serial.print("OptaWifiSupport: beginAP() returned status ");
    Serial.println(status);
  }
  return status == WL_AP_LISTENING || status == WL_AP_CONNECTED;
}

bool maintainAccessPoint() {
  if (!haveApSettings) {
    return false;
  }

  int status = WiFi.status();
  if (status == WL_AP_LISTENING || status == WL_AP_CONNECTED) {
    return true;
  }

  return beginAccessPoint(apSsid.c_str(), apPassword.c_str(), apIp, apSubnet);
}

bool beginStation(const char* ssid, const char* password, IPAddress ip, IPAddress subnet, IPAddress gateway) {
  haveStationSettings = true;
  stationSsid = ssid;
  stationPassword = password;
  stationIp = ip;
  stationSubnet = subnet;
  stationGateway = gateway;

  WiFi.config(ip, IPAddress(), gateway, subnet);
  WiFi.begin(ssid, password);

  // begin() doesn't reflect the final outcome immediately - the actual handshake takes a
  // moment, so status() must be polled with a short delay rather than checked once right
  // after begin() returns.
  const int MAX_ATTEMPTS = 10;
  for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
  }

  Serial.print("OptaWifiSupport: did not reach WL_CONNECTED after retries, status ");
  Serial.println(WiFi.status());
  return false;
}

bool isStationConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool maintainStationConnection() {
  if (isStationConnected()) {
    return true;
  }

  if (!haveStationSettings) {
    return false;
  }

  return beginStation(stationSsid.c_str(), stationPassword.c_str(), stationIp, stationSubnet, stationGateway);
}
