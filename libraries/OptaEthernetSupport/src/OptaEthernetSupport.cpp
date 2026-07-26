#include "OptaEthernetSupport.h"
#include <Ethernet.h>
#include <string.h>

// Confirmed against the actual installed Opta core (arduino:mbed_opta 4.6.0): EthernetClass
// (Ethernet.h) configures a static IP directly via
// begin(mac, ip, dns, gateway, subnet, ...) - unlike WiFi, there's no separate config() step.
// Physical link state is reported by linkStatus() (LinkON/LinkOFF/Unknown), which is what's
// checked below rather than begin()'s return value - the header only documents a return-value
// meaning for the DHCP overload, not the static-IP one, so linkStatus() is the more trustworthy
// signal here.

namespace {

bool haveSettings = false;
uint8_t savedMac[6];
IPAddress savedIp, savedSubnet, savedGateway;

}  // namespace

bool beginEthernet(const uint8_t mac[6], IPAddress ip, IPAddress subnet, IPAddress gateway) {
  haveSettings = true;
  memcpy(savedMac, mac, 6);
  savedIp = ip;
  savedSubnet = subnet;
  savedGateway = gateway;

  // Ethernet.begin() takes a non-const uint8_t* even though it doesn't modify the buffer.
  Ethernet.begin(const_cast<uint8_t*>(mac), ip, IPAddress(), gateway, subnet);

  EthernetLinkStatus linkStatus = Ethernet.linkStatus();
  if (linkStatus != LinkON) {
    Serial.print("OptaEthernetSupport: link status after begin() is ");
    Serial.println(linkStatus == LinkOFF ? "LinkOFF" : "Unknown");
  }
  return linkStatus == LinkON;
}

bool isEthernetConnected() {
  return Ethernet.linkStatus() == LinkON;
}

bool maintainEthernetConnection() {
  if (isEthernetConnected()) {
    return true;
  }

  if (!haveSettings) {
    return false;
  }

  return beginEthernet(savedMac, savedIp, savedSubnet, savedGateway);
}
