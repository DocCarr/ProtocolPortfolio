#ifndef OPTA_ETHERNET_SUPPORT_H
#define OPTA_ETHERNET_SUPPORT_H

#include <Arduino.h>
#include <IPAddress.h>

// Brings up the Ethernet interface with the given MAC/IP/subnet/gateway.
bool beginEthernet(const uint8_t mac[6], IPAddress ip, IPAddress subnet, IPAddress gateway);

// Returns true if the Ethernet link is currently up.
bool isEthernetConnected();

// If the Ethernet link has dropped, attempts to bring it back up using the settings
// supplied to the most recent beginEthernet() call.
bool maintainEthernetConnection();

#endif
