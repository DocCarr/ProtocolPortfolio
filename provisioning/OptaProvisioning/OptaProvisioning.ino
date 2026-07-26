// OptaProvisioning.ino
// Top-down skeleton: setup()/loop() call stubs, filled in bottom-up.
// Generic filesystem and WiFi mechanics live in libraries/ (see each library's header).
// HTTP handling and file-handling policy stay here, since only this sketch uses them.
//
// There is no WebServer-style HTTP routing library available for this board (confirmed
// absent from the installed Opta core and every other installed package/sketchbook
// library) - only WiFiServer/WiFiClient (raw TCP accept/read/write, from the same WiFi
// library as OptaWifiSupport uses). handleWebServerRequests() parses the raw HTTP request
// line/headers/body itself from the accepted WiFiClient, rather than registering routes
// with a framework.

#include <OptaConfigStorage.h>
#include <OptaWifiSupport.h>
#include <WiFiServer.h>

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

// Required field paths per docs/network-config-schema.md and docs/mapping-config-schema.md.
// "registers" is checked for presence only - hasRequiredFields() checks a single fixed path,
// not per-element fields inside an array, which is a good-enough test-harness simplification.
const char* NETWORK_REQUIRED_FIELDS[] = {
  "ethernet.mac", "ethernet.ip", "ethernet.subnetMask", "ethernet.gateway",
  "wifi.ssid", "wifi.password", "wifi.ip", "wifi.subnetMask", "wifi.gateway"
};
const size_t NETWORK_REQUIRED_FIELD_COUNT = sizeof(NETWORK_REQUIRED_FIELDS) / sizeof(NETWORK_REQUIRED_FIELDS[0]);

const char* MAPPING_REQUIRED_FIELDS[] = {
  "modbus.targetIp", "modbus.targetPort",
  "mqtt.brokerIp", "mqtt.brokerPort", "mqtt.clientId",
  "pollIntervalMs", "registers"
};
const size_t MAPPING_REQUIRED_FIELD_COUNT = sizeof(MAPPING_REQUIRED_FIELDS) / sizeof(MAPPING_REQUIRED_FIELDS[0]);

// ---- Globals ----

WiFiServer server(WEBSERVER_PORT);

// ---- Declarations ----

void setupFilePartition();
void setupWifiAccessPoint();
void setupWebServer();

void handleWebServerRequests();
void sendResponse(WiFiClient& client, int statusCode, const char* statusText, const String& body);

void handleGetNetworkConfig(WiFiClient& client);
void handleDeleteNetworkConfig(WiFiClient& client);
void handlePushNetworkConfig(WiFiClient& client, const String& body);

void handleGetMappingConfig(WiFiClient& client);
void handleDeleteMappingConfig(WiFiClient& client);
void handlePushMappingConfig(WiFiClient& client, const String& body);

bool validateJson(const String& json, const char* requiredFieldPaths[], size_t count);

// Blocks reading Serial for a single 'y'/'n' answer, ignoring anything else (e.g. the
// newline after a keystroke). Matches the confirm-before-format prompt style used in
// Arduino's own QSPIFormat.ino example.
bool waitYesNo();

// ---- setup/loop ----

void setup() {
  Serial.begin(9600);
  while (!Serial);  // block until Serial Monitor actually connects - this sketch is
                    // interactive (needs y/n input), so there's no harm waiting here,
                    // unlike the bridge sketch which must run unattended.

  setupFilePartition();
  setupWifiAccessPoint();
  setupWebServer();
}

void loop() {
  maintainAccessPoint();
  handleWebServerRequests();
}

// ---- Bodies ----

void setupFilePartition() {
  MountResult result = mountPartition(USER_PARTITION_INDEX);
  if (result == MountResult::Mounted) {
    Serial.println("User partition mounted.");
    return;
  }

  Serial.println("User partition did not mount (not yet formatted, or an error).");
  Serial.println("Type 'y' and press Enter to format it now, or 'n' to halt without formatting.");
  if (waitYesNo()) {
    if (formatAndMountPartition(USER_PARTITION_INDEX)) {
      Serial.println("User partition formatted and mounted.");
    } else {
      Serial.println("Formatting failed. Halting.");
      while (true) {
        delay(1000);
      }
    }
  } else {
    Serial.println("Not formatting. Halting - reset the board once ready to proceed.");
    while (true) {
      delay(1000);
    }
  }
}

bool waitYesNo() {
  while (true) {
    if (Serial.available()) {
      char choice = Serial.read();
      if (choice == 'y' || choice == 'Y') {
        return true;
      }
      if (choice == 'n' || choice == 'N') {
        return false;
      }
      // ignore anything else (e.g. the trailing newline) and keep waiting
    }
  }
}

void setupWifiAccessPoint() {
  if (beginAccessPoint(AP_SSID, AP_PASSWORD, AP_IP, AP_SUBNET)) {
    Serial.println("Access point started.");
  } else {
    Serial.println("Failed to start access point.");
  }
}

void setupWebServer() {
  server.begin();
  Serial.print("Web server listening on port ");
  Serial.println(WEBSERVER_PORT);
}

void handleWebServerRequests() {
  WiFiClient client = server.accept();
  if (!client) {
    return;
  }

  String requestLine = client.readStringUntil('\n');
  requestLine.trim();

  int firstSpace = requestLine.indexOf(' ');
  int secondSpace = requestLine.indexOf(' ', firstSpace + 1);
  if (firstSpace == -1 || secondSpace == -1) {
    client.stop();
    return;
  }

  String method = requestLine.substring(0, firstSpace);
  String path = requestLine.substring(firstSpace + 1, secondSpace);

  int contentLength = 0;
  while (client.connected()) {
    String headerLine = client.readStringUntil('\n');
    headerLine.trim();
    if (headerLine.length() == 0) {
      break;
    }
    if (headerLine.startsWith("Content-Length:")) {
      contentLength = headerLine.substring(headerLine.indexOf(':') + 1).toInt();
    }
  }

  String body;
  if (contentLength > 0) {
    body.reserve(contentLength);
    char buffer[128];
    int remaining = contentLength;
    while (remaining > 0) {
      int chunk = remaining < (int)sizeof(buffer) ? remaining : (int)sizeof(buffer);
      int bytesRead = client.readBytes(buffer, chunk);
      if (bytesRead <= 0) {
        break;
      }
      body.concat(buffer, bytesRead);
      remaining -= bytesRead;
    }
  }

  if (path == "/network.json") {
    if (method == "GET") {
      handleGetNetworkConfig(client);
    } else if (method == "DELETE") {
      handleDeleteNetworkConfig(client);
    } else if (method == "POST") {
      handlePushNetworkConfig(client, body);
    } else {
      sendResponse(client, 405, "Method Not Allowed", "");
    }
  } else if (path == "/mapping.json") {
    if (method == "GET") {
      handleGetMappingConfig(client);
    } else if (method == "DELETE") {
      handleDeleteMappingConfig(client);
    } else if (method == "POST") {
      handlePushMappingConfig(client, body);
    } else {
      sendResponse(client, 405, "Method Not Allowed", "");
    }
  } else {
    sendResponse(client, 404, "Not Found", "");
  }

  client.stop();
}

void sendResponse(WiFiClient& client, int statusCode, const char* statusText, const String& body) {
  client.print("HTTP/1.1 ");
  client.print(statusCode);
  client.print(" ");
  client.println(statusText);
  client.print("Content-Length: ");
  client.println(body.length());
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  if (body.length() > 0) {
    client.print(body);
  }
}

void handleGetNetworkConfig(WiFiClient& client) {
  String contents;
  if (readFile("network.json", contents)) {
    sendResponse(client, 200, "OK", contents);
  } else {
    sendResponse(client, 404, "Not Found", "");
  }
}

void handleDeleteNetworkConfig(WiFiClient& client) {
  if (deleteFile("network.json")) {
    sendResponse(client, 200, "OK", "");
  } else {
    sendResponse(client, 404, "Not Found", "");
  }
}

void handlePushNetworkConfig(WiFiClient& client, const String& body) {
  // Test-harness flow: write the raw body directly, then read it back and validate - see the
  // note in TODO.md on why a production provisioning path should validate before writing.
  if (!writeFile("network.json", body)) {
    Serial.println("network.json: write failed.");
    sendResponse(client, 500, "Internal Server Error", "");
    return;
  }

  String writtenContents;
  bool valid = readFile("network.json", writtenContents) &&
               validateJson(writtenContents, NETWORK_REQUIRED_FIELDS, NETWORK_REQUIRED_FIELD_COUNT);

  if (valid) {
    Serial.println("network.json: received and validated OK.");
    sendResponse(client, 200, "OK", "");
  } else {
    Serial.println("network.json: received but FAILED validation.");
    sendResponse(client, 422, "Unprocessable Entity", "");
  }
}

void handleGetMappingConfig(WiFiClient& client) {
  String contents;
  if (readFile("mapping.json", contents)) {
    sendResponse(client, 200, "OK", contents);
  } else {
    sendResponse(client, 404, "Not Found", "");
  }
}

void handleDeleteMappingConfig(WiFiClient& client) {
  if (deleteFile("mapping.json")) {
    sendResponse(client, 200, "OK", "");
  } else {
    sendResponse(client, 404, "Not Found", "");
  }
}

void handlePushMappingConfig(WiFiClient& client, const String& body) {
  // Test-harness flow: write the raw body directly, then read it back and validate - see the
  // note in TODO.md on why a production provisioning path should validate before writing.
  if (!writeFile("mapping.json", body)) {
    Serial.println("mapping.json: write failed.");
    sendResponse(client, 500, "Internal Server Error", "");
    return;
  }

  String writtenContents;
  bool valid = readFile("mapping.json", writtenContents) &&
               validateJson(writtenContents, MAPPING_REQUIRED_FIELDS, MAPPING_REQUIRED_FIELD_COUNT);

  if (valid) {
    Serial.println("mapping.json: received and validated OK.");
    sendResponse(client, 200, "OK", "");
  } else {
    Serial.println("mapping.json: received but FAILED validation.");
    sendResponse(client, 422, "Unprocessable Entity", "");
  }
}

bool validateJson(const String& json, const char* requiredFieldPaths[], size_t count) {
  return isWellFormedJson(json) && hasRequiredFields(json, requiredFieldPaths, count);
}
