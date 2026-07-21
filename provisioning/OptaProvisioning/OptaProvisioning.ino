// OptaProvisioning.ino
// Top-down skeleton: setup()/loop() call stubs, filled in bottom-up.
// Generic filesystem and WiFi mechanics live in libraries/ (see each library's header).
// WebServer routing and file-handling policy stay here, since only this sketch uses them.

#include <OptaConfigStorage.h>
#include <OptaWifiSupport.h>
#include <WebServer.h>

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

void handleWebServerRequests();

void handleGetNetworkConfig();
void handleDeleteNetworkConfig();
void handlePushNetworkConfig();

void handleGetMappingConfig();
void handleDeleteMappingConfig();
void handlePushMappingConfig();

bool validateJson(const String& json, const char* requiredFieldPaths[], size_t count);

// ---- setup/loop ----

void setup() {
  Serial.begin(9600);

  setupFilePartition();
  setupWifiAccessPoint();
  setupWebServer();
}

void loop() {
  maintainAccessPoint();
  handleWebServerRequests();
}

// ---- Stub bodies (to be implemented bottom-up) ----

void setupFilePartition() {
  // Mount USER_PARTITION_INDEX via OptaConfigStorage::mountPartition(). On a NotFormatted or
  // Error result, wait for an explicit confirmation (e.g. a serial command) before calling
  // OptaConfigStorage::formatAndMountPartition() - never auto-format, so a transient mount
  // failure can't silently wipe already-provisioned config.
}

void setupWifiAccessPoint() {
  // OptaWifiSupport::beginAccessPoint() using AP_SSID/AP_PASSWORD/AP_IP/AP_SUBNET.
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

void handleWebServerRequests() {
  // Dispatch pending client requests to the registered routes.
}

void handleGetNetworkConfig() {
  // OptaConfigStorage::readFile("network.json", ...) and return it.
}

void handleDeleteNetworkConfig() {
  // OptaConfigStorage::deleteFile("network.json").
}

void handlePushNetworkConfig() {
  // Test-harness flow: OptaConfigStorage::writeFile("network.json", body) directly, then
  // read it back and call validateJson() with network.json's required field paths, logging
  // a pass/fail message to Serial. A production provisioning path should validate before
  // writing instead - see the note in TODO.md.
}

void handleGetMappingConfig() {
  // OptaConfigStorage::readFile("mapping.json", ...) and return it.
}

void handleDeleteMappingConfig() {
  // OptaConfigStorage::deleteFile("mapping.json").
}

void handlePushMappingConfig() {
  // Test-harness flow: OptaConfigStorage::writeFile("mapping.json", body) directly, then
  // read it back and call validateJson() with mapping.json's required field paths, logging
  // a pass/fail message to Serial. A production provisioning path should validate before
  // writing instead - see the note in TODO.md.
}

bool validateJson(const String& json, const char* requiredFieldPaths[], size_t count) {
  // Thin wrapper over OptaConfigStorage::isWellFormedJson()/hasRequiredFields() - this
  // sketch supplies the required-field-path list for whichever file is being checked.
  return false;
}
