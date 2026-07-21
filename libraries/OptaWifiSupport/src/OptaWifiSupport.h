#ifndef OPTA_WIFI_SUPPORT_H
#define OPTA_WIFI_SUPPORT_H

#include <Arduino.h>
#include <IPAddress.h>

// Access point mode (hosting a network) and station mode (joining a network) are different
// operations - they are not interchangeable, even though both are "WiFi setup."

// Brings up a WiFi access point with the given SSID/password and static IP/subnet.
bool beginAccessPoint(const char* ssid, const char* password, IPAddress ip, IPAddress subnet);

// If the access point has dropped (e.g. a radio reset), attempts to restart it using the
// settings supplied to the most recent beginAccessPoint() call.
bool maintainAccessPoint();

// Connects to an existing WiFi network in station mode with a static IP.
bool beginStation(const char* ssid, const char* password, IPAddress ip, IPAddress subnet, IPAddress gateway);

// Returns true if the station connection is currently up.
bool isStationConnected();

// If the station connection has dropped, attempts to reconnect using the credentials
// supplied to the most recent beginStation() call. No-op (returns true) if already connected.
bool maintainStationConnection();

#endif
