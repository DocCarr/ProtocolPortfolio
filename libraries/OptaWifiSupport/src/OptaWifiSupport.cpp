#include "OptaWifiSupport.h"

// Top-down skeleton - bodies filled in bottom-up.

bool beginAccessPoint(const char* ssid, const char* password, IPAddress ip, IPAddress subnet) {
  // Bring up the access point (e.g. WiFi.beginAP()) and configure its static IP/subnet.
  return false;
}

bool maintainAccessPoint() {
  // Check whether the access point is still active; restart it with the last-supplied
  // settings if it has dropped.
  return false;
}

bool beginStation(const char* ssid, const char* password, IPAddress ip, IPAddress subnet, IPAddress gateway) {
  // Connect to the network (e.g. WiFi.begin()) and configure the static IP/subnet/gateway.
  return false;
}

bool isStationConnected() {
  // Return whether the station connection is currently up.
  return false;
}

bool maintainStationConnection() {
  // If not connected, reattempt beginStation() with the last-supplied credentials/settings.
  return false;
}
