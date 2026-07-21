// OptaProvisioning.ino
// Top-down skeleton: setup()/loop() call stubs, filled in bottom-up.
// Library/API names below are placeholders for the concrete provisioning APIs and may
// need to change once bottom-up implementation confirms what's actually available.

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <QSPIFBlockDevice.h>
#include <MBRBlockDevice.h>

// ---- Configuration ----

// Provisioning access point credentials - PLACEHOLDERS.
// Update manually at runtime before each use; never commit real values here.
// See tests/procedures/TEMPLATE.md for the pre-test reminder.
const char* AP_SSID = "REPLACE_WITH_AP_SSID";
const char* AP_PASSWORD = "REPLACE_WITH_AP_PASSWORD";

const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

const int WEBSERVER_PORT = 80;

// LittleFS user partition - see docs/partitioning-notes.md (partition 4, offset 7MB, size 7MB).
const uint32_t USER_PARTITION_INDEX = 4;

// ---- Globals ----

WebServer server(WEBSERVER_PORT);

// ---- Stub declarations ----

void setupFilePartition();
void setupWifiAccessPoint();
void setupWebServer();

void handleWifiReconnect();
void handleWebServerRequests();

void handleGetNetworkConfig();
void handleDeleteNetworkConfig();
void handlePushNetworkConfig();

void handleGetMappingConfig();
void handleDeleteMappingConfig();
void handlePushMappingConfig();

bool validateJson(const String& json);

// ---- setup/loop ----

void setup() {
  Serial.begin(9600);

  setupFilePartition();
  setupWifiAccessPoint();
  setupWebServer();
}

void loop() {
  handleWifiReconnect();
  handleWebServerRequests();
}

// ---- Stub bodies (to be implemented bottom-up) ----

void setupFilePartition() {
  // Mount the LittleFS user partition (MBRBlockDevice at USER_PARTITION_INDEX, over
  // QSPIFBlockDevice). On mount failure, do NOT auto-format - wait for an explicit
  // confirmation (e.g. a serial command) before formatting, so a transient mount
  // failure can't silently wipe already-provisioned config.
}

void setupWifiAccessPoint() {
  // Bring up the access point using AP_SSID/AP_PASSWORD and AP_IP/AP_SUBNET.
}

void setupWebServer() {
  // Test harness only - no upload form. Config files are sent directly via curl, e.g.:
  //   curl --data-binary @network.json http://<opta-ip>/network.json
  // Register routes:
  //   GET    /network.json  -> handleGetNetworkConfig
  //   DELETE /network.json  -> handleDeleteNetworkConfig
  //   POST   /network.json  -> handlePushNetworkConfig
  //   GET    /mapping.json  -> handleGetMappingConfig
  //   DELETE /mapping.json  -> handleDeleteMappingConfig
  //   POST   /mapping.json  -> handlePushMappingConfig
  // network.json and mapping.json are handled independently of each other.
}

void handleWifiReconnect() {
  // Detect a dropped access point and restart it.
}

void handleWebServerRequests() {
  // Dispatch pending client requests to the registered routes.
}

void handleGetNetworkConfig() {
  // Read network.json from the user partition and return it.
}

void handleDeleteNetworkConfig() {
  // Delete network.json from the user partition.
}

void handlePushNetworkConfig() {
  // Test-harness flow: write the raw request body directly to network.json, then read it
  // back, run validateJson() against the loaded content, and log a pass/fail message to
  // Serial. A production provisioning path should validate before writing instead - see
  // the note in TODO.md.
}

void handleGetMappingConfig() {
  // Read mapping.json from the user partition and return it.
}

void handleDeleteMappingConfig() {
  // Delete mapping.json from the user partition.
}

void handlePushMappingConfig() {
  // Test-harness flow: write the raw request body directly to mapping.json, then read it
  // back, run validateJson() against the loaded content, and log a pass/fail message to
  // Serial. A production provisioning path should validate before writing instead - see
  // the note in TODO.md.
}

bool validateJson(const String& json) {
  // Confirm well-formed JSON and required fields are present.
  return false;
}
