#include "OptaEthernetSupport.h"

// Top-down skeleton - bodies filled in bottom-up.

bool beginEthernet(const uint8_t mac[6], IPAddress ip, IPAddress subnet, IPAddress gateway) {
  // Bring up the Ethernet interface (e.g. Ethernet.begin()) with the given static settings.
  return false;
}

bool isEthernetConnected() {
  // Return whether the Ethernet link is currently up.
  return false;
}

bool maintainEthernetConnection() {
  // If the link is down, reattempt beginEthernet() with the last-supplied settings.
  return false;
}
